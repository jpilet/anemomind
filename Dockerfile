
# C++ Anemomind server MULTI STAGE BUILD
FROM debian:10-slim as cppbuilder

ARG LOAD_MONGO_DB
ARG MONGO_URL
ENV LOAD_MONGO_DB ${LOAD_MONGO_DB:-false}
ENV MONGO_URL ${MONGO_URL}

#  Install packages
#  Clean up APT when done.
RUN apt-get update && \
	apt-get install --no-install-recommends -y base-files  bootlogd build-essential \
    ca-certificates catdoc clang cmake dmidecode dstat ethstatus f2c file fio gdb git \
    gnupg grub-efi-amd64 haveged htop icu-devtools libarmadillo-dev libatlas3-base \
    libblas-dev libblocksruntime-dev libboost-dev libboost-filesystem-dev libboost-iostreams-dev \
    libboost-regex-dev libboost-system-dev libboost-thread-dev libbsd-dev libcairo2-dev \
    libcurl4-openssl-dev libcxsparse3 libedit-dev libffi6 libgmp10 libgnutls30 \
    libhogweed4 libicu-dev libicu63 libidn11 libidn2-0 liblapack-dev libncurses5-dev libnettle6 \
    libnss-myhostname libp11-kit0 libprotobuf-dev libpsl5 libsqlite3-dev libssl-dev libsuitesparse-dev \
    libsystemd0 libtasn1-6 libudev1 libunistring2 libxml2-dev locales locate lsb-release lsof make \
    man-db mg mosh mutt net-tools netcat ninja-build pkg-config protobuf-compiler pstack rsync rsyslog \
    shunit2 socat ssh swift swig sysstat systemd-sysv systemtap-sdt-dev tcpdump tinc tmux traceroute \
    tzdata unattended-upgrades uuid-dev uuid-runtime wget whiptail xfsprogs libeigen3-dev && \
    apt-get clean autoclean && \
    apt-get autoremove -y && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Copy cmake structure, without source code so that we can build and cache
# dependencies
COPY ./cmake /anemomind/cmake
COPY ./CMakeLists.txt /anemomind/CMakeLists.txt
RUN mkdir /anemomind/src && echo > /anemomind/src/CMakeLists.txt

RUN mkdir -p /anemomind/build
WORKDIR /anemomind/build
RUN cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo /anemomind
RUN make -j4 poco_ext gtest_ext ceres_ext

#copy all the code related to C++ : use .dockerignore for unwanted files
RUN rm -rf /anemomind/src
COPY ./src /anemomind/src

# compile link and copy the bin with dependencies to /tmp/
# we need to do it as one RUN command since the products of the workdir are temp
# for this intermediate container (layer)
# Use of J1 can be an arg ?

#RUN cmake .. -DCMAKE_BUILD_TYPE=RelWidthDebInfo \
#    && make -j1 nautical_processBoatLogs logimport_summary anemobox_logcat logimport_try_load nautical_catTargetSpeed

RUN make rebuild_cache && make -j4 nautical_processBoatLogs logimport_summary anemobox_logcat logimport_try_load nautical_catTargetSpeed

WORKDIR /anemomind/build
RUN mkdir -p /temp/lib \
    && mkdir -p /temp/bin \
    && cp $(ldd ./src/server/nautical/nautical_catTargetSpeed \
        ./src/server/nautical/nautical_processBoatLogs \
        ./src/server/nautical/logimport/logimport_try_load | grep -o '/.\+\.so[^ ]*' | sort | uniq) /temp/lib \
    && cp /anemomind/build/src/server/nautical/nautical_processBoatLogs /temp/bin \
    && cp /anemomind/build/src/server/nautical/nautical_catTargetSpeed /temp/bin \
    && cp /anemomind/build/src/server/nautical/logimport/logimport_try_load /temp/bin \
    && service ssh start

COPY ./src/server/production/* /temp/bin/

# optionally load mongo db
#RUN if [ "$LOAD_MONGO_DB" = true ] ; then \
#    echo 'attempting to load data to mongodb' && \
#    chmod +x ./src/server/nautical/tiles/generateDevDB.sh && \
#    ./src/server/nautical/tiles/generateDevDB.sh; \
#    fi

RUN echo 'PermitRootLogin yes' >> /etc/ssh/sshd_config && \
    echo 'PermitEmptyPasswords yes' >> /etc/ssh/sshd_config && \
    echo 'PasswordAuthentication yes' >> /etc/ssh/sshd_config && \
    ssh-keygen -A

EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]

# now start from a lean image and copy all the needed bin/libs only
FROM debian:10-slim

ARG MONGO_URL
ENV MONGO_URL ${MONGO_URL}

WORKDIR /anemomind

RUN apt-get update && apt-get install  --no-install-recommends -y cron gnupg
ADD https://www.mongodb.org/static/pgp/server-4.2.asc mongo.asc
RUN apt-key add mongo.asc && rm mongo.asc
RUN echo "deb http://repo.mongodb.org/apt/debian buster/mongodb-org/4.2 main" > /etc/apt/sources.list.d/mongodb-org-4.2.list && \
    apt-get update && \
    apt-get install -y mongodb-org-shell && apt-get autoclean && apt-get clean


# app
COPY --from=cppbuilder /temp/lib/* /anemomind/lib/
COPY --from=cppbuilder /temp/bin/* /anemomind/bin/

ENV LD_LIBRARY_PATH="/anemomind/lib/"

#WORKDIR /anemomind/bin

RUN env | grep -v "=$" | grep -v "_=" > /etc/cron.d/anemo-cron && \
    echo "*/3    *    *   *   *    flock -n /tmp/process_logs.lock /anemomind/bin/processNewLogs_DockerDev.sh >> /var/log/cron.log 2>&1" >> /etc/cron.d/anemo-cron && \
    chmod 0644 /etc/cron.d/anemo-cron

# Apply cron job
RUN crontab /etc/cron.d/anemo-cron

# Create the log file to be able to run tail
RUN touch /var/log/cron.log

# Run the command on container startup
CMD cron && tail -f /var/log/cron.log


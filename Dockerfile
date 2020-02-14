
# C++ Anemomind server MULTI STAGE BUILD
FROM debian:10-slim as build_env

ARG LOAD_MONGO_DB
ARG MONGO_URL
ENV LOAD_MONGO_DB ${LOAD_MONGO_DB:-false}
ENV MONGO_URL ${MONGO_URL}

#  Install packages
#  Clean up APT when done.
RUN apt-get update && \
	apt-get install --no-install-recommends -y base-files  bootlogd build-essential \
    ca-certificates cmake dmidecode dstat ethstatus f2c file fio gdb git \
    gnupg grub-efi-amd64 haveged htop icu-devtools libarmadillo-dev libatlas3-base \
    libblas-dev libblocksruntime-dev libboost-dev libboost-filesystem-dev libboost-iostreams-dev \
    libboost-regex-dev libboost-system-dev libboost-thread-dev libbsd-dev libcairo2-dev \
    libcurl4-openssl-dev libcxsparse3 libedit-dev libffi6 libgmp10 libgnutls30 \
    libhogweed4 libicu-dev libicu63 libidn11 libidn2-0 liblapack-dev libncurses5-dev libnettle6 \
    libnss-myhostname libp11-kit0 libprotobuf-dev libpsl5 libsqlite3-dev libssl-dev libsuitesparse-dev \
    libsystemd0 libtasn1-6 libudev1 libunistring2 libxml2-dev locales locate lsb-release lsof make \
    net-tools netcat ninja-build pkg-config protobuf-compiler pstack rsync rsyslog \
    ssh sysstat systemd-sysv systemtap-sdt-dev tinc tmux traceroute \
    tzdata unattended-upgrades uuid-dev uuid-runtime wget whiptail xfsprogs libeigen3-dev && \
    apt-get clean autoclean && \
    apt-get autoremove -y && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

FROM build_env as basebuild

# Copy cmake structure, without source code so that we can build and cache
# dependencies
COPY ./cmake /anemomind/cmake
COPY ./CMakeLists.txt /anemomind/CMakeLists.txt
RUN mkdir /anemomind/src && echo > /anemomind/src/CMakeLists.txt

RUN mkdir -p /anemomind/build
WORKDIR /anemomind/build
RUN cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo /anemomind

# build prod dependencies only
RUN make -j$(nproc) poco_ext gtest_ext ceres_ext mongo_ext adolc_ext

#copy all the code related to C++ : use .dockerignore for unwanted files
RUN rm -rf /anemomind/src
COPY ./src /anemomind/src
RUN make rebuild_cache

# compile and link prod binaries.
RUN make -j$(nproc) nautical_processBoatLogs logimport_summary \
    anemobox_logcat logimport_try_load nautical_catTargetSpeed

# to compile and execute unit tests, do:
# docker build --target=unit_tester .
FROM basebuild as unit_tester

WORKDIR /anemomind/build
RUN make -j$(nproc) && make -j$(nproc) test

FROM basebuild as anemomind_prod

# Copy the prod bins with dependencies to /tmp/
# we need to do it as one RUN command since the products of the workdir are
# temp for this intermediate container (layer)
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
# build with docker build --target=anemomind_prod
FROM debian:10-slim as anemomind_prod

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


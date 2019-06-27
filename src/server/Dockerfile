
# C++ Anemomind server MULTI STAGE BUILD
FROM debian:9.8-slim as cppbuilder

ARG LOAD_MONGO_DB
ARG MONGO_URL
ENV LOAD_MONGO_DB ${LOAD_MONGO_DB:-false}
ENV MONGO_URL ${MONGO_URL}

WORKDIR "/anemolab/src/server"

#  Install packages
#  Clean up APT when done.
RUN apt-get update && \
	apt-get install --no-install-recommends -y base-files  bootlogd bsdutils build-essential \
    ca-certificates catdoc clang cmake coreutils curl dash dbus debconf debian-archive-keyring \
    debianutils diffutils dmidecode dpkg dstat e2fslibs e2fsprogs ethstatus f2c file findutils \
    fio gcc-6-base gdb git gnupg gpgv grep grub-efi-amd64 gzip haveged htop icu-devtools \
    ifupdown init-system-helpers ioping iotop iperf iptables iputils-ping isc-dhcp-client kmod \
    libacl1 libapt-pkg5.0 libarmadillo-dev libatlas3-base libattr1 libaudit-common libaudit1 \
    libblas-dev libblkid1 libblocksruntime-dev libboost-dev libboost-filesystem-dev \
    libboost-iostreams-dev libboost-regex-dev libboost-system-dev libboost-thread-dev libbsd-dev \
    libbz2-1.0 libc-bin libc6 libcairo2-dev libcap-ng0 libcomerr2 libcurl4-openssl-dev libcxsparse3 \
    libdb5.3 libdebconfclient0 libedit-dev  libfdisk1 libffi6 libgcc1 libgcrypt20 libgmp10 libgnutls30 \
    libgpg-error0 libhogweed4 libicu-dev libicu57 libidn11 libidn2-0 liblapack-dev liblz4-1 liblzma5 \
    libmount1 libncurses5-dev libncursesw5 libnettle6 libnss-myhostname libp11-kit0 libpam-modules \
    libpam-modules-bin libpam-runtime libpam0g libpcre3 libprotobuf-dev libpsl5 libpython-dev \
    libpython2.7 libselinux1 libsemanage-common libsemanage1 libsepol1 libsmartcols1 libsqlite3-dev\
    libss2 libssl-dev libstdc++6 libsuitesparse-dev libsystemd0 libtasn1-6 libtinfo5 libudev1 \
    libunistring0 libustr-1.0-1 libuuid1 libxml2-dev linux-image-amd64 linux-perf-4.9 locales \
    locate lsb-base lsb-release lsof make man-db mawk mg mosh mount multiarch-support mutt ncurses-base \
    ncurses-bin net-tools netcat ninja-build ntp ntpdate perl-base pkg-config protobuf-compiler pstack \
    rsync rsyslog screen sed sensible-utils shunit2 socat ssh swift swig sysstat systemd-sysv \
    systemtap-sdt-dev sysvinit-utils tar tcpdump tinc tmux traceroute tzdata unattended-upgrades \
    util-linux uuid-dev uuid-runtime wget whiptail xfsprogs zlib1g libeigen3-dev && \
    apt-get clean autoclean && \
    apt-get autoremove -y && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*


#copy all the code related to C++ : use .dockerignore for unwanted files
COPY ./src /anemolab/src
COPY ./cmake /anemolab/cmake
COPY ./CMakeLists.txt /anemolab/CMakeLists.txt

RUN mkdir -p /anemolab/build 
WORKDIR "/anemolab/build"
# compile link and copy the bin with dependencies to /tmp/
# we need to do it as one RUN command since the products of the workdir are temp
# for this intermediate container (layer)
# Use of J1 can be an arg ? 
RUN cmake .. -DCMAKE_BUILD_TYPE=RelWidthDebInfo \ 
    && make -j1 nautical_processBoatLogs logimport_summary anemobox_logcat logimport_try_load nautical_catTargetSpeed

RUN mkdir -p /temp/lib \
    && mkdir -p /temp/bin \
    && cp $(ldd ./src/server/nautical/nautical_catTargetSpeed \
        ./src/server/nautical/nautical_processBoatLogs \
        ./src/server/nautical/logimport/logimport_try_load | grep -o '/.\+\.so[^ ]*' | sort | uniq) /temp/lib \
    && cp /anemolab/build/src/server/nautical/nautical_processBoatLogs /temp/bin \
    && cp /anemolab/build/src/server/nautical/nautical_catTargetSpeed /temp/bin \
    && cp /anemolab/build/src/server/nautical/logimport/logimport_try_load /temp/bin

COPY ./src/server/production/* /temp/bin/

# optionally load mongo db
RUN if [ "$LOAD_MONGO_DB" = true ] ; then \
    echo 'attempting to load data to mongodb' && \
    chmod +x ./src/server/nautical/tiles/generateDevDB.sh && \
    ./src/server/nautical/tiles/generateDevDB.sh; \
    fi

# now start from a lean image and copy all the needed bin/libs only
FROM debian:9.8-slim

ARG MONGO_URL
ENV MONGO_URL ${MONGO_URL}

WORKDIR /anemomind

RUN apt-get update   --no-install-recommends && \
    apt-get install  --no-install-recommends -y cron mongodb-clients && \
    apt-get autoclean && apt-get clean


# app
COPY --from=cppbuilder /temp/lib/* /anemomind/lib/
COPY --from=cppbuilder /temp/bin/* /anemomind/bin/

ENV LD_LIBRARY_PATH="/anemomind/lib/"

# Install GCSFuse - this layer is kept separate as it is temporary - till components start using unified storage api
ENV GCSFUSE_REPO gcsfuse-stretch

RUN apt-get update && apt-get install --yes --no-install-recommends \
    ca-certificates \
    curl \
    gnupg \
  && echo "deb http://packages.cloud.google.com/apt $GCSFUSE_REPO main" \
    | tee /etc/apt/sources.list.d/gcsfuse.list \
  && curl https://packages.cloud.google.com/apt/doc/apt-key.gpg | apt-key add - \
  && apt-get update \
  && apt-get install --yes gcsfuse \
  && apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# configure boat logs & gcsfuse
RUN mkdir -p /anemomind/boat_logs


WORKDIR /anemomind/bin 

RUN echo "* *  *   *   * flock -n /tmp/process_logs.lock /anemomind/bin/processNewLogs_DockerDev.sh >> /var/log/cron.log" >> /etc/cron.d/anemo-cron && \
    echo >>/etc/cron.d/anemo-cron && \
    chmod 0644 /etc/cron.d/anemo-cron && \
    chmod -R 755 .

# Apply cron job
RUN crontab /etc/cron.d/anemo-cron

# Create the log file to be able to run tail
RUN touch /var/log/cron.log

# Run the command on container startup
# CMD cron && tail -f /var/log/cron.log
# ENTRYPOINT ["./run-cron.sh"]
CMD ./run-cron.sh && tail -f /var/log/cron.log


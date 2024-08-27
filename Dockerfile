# C++ Anemomind server MULTI STAGE BUILD
FROM debian:bullseye-slim as build_env

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
    libcurl4-openssl-dev libcxsparse3 libedit-dev libgmp10 libgnutls30 \
    libicu-dev libidn11 libidn2-0 liblapack-dev libncurses5-dev \
    libnss-myhostname libp11-kit0 libprotobuf-dev libpsl5 libsqlite3-dev libssl-dev libsuitesparse-dev \
    libsystemd0 libtasn1-6 libudev1 libunistring2 libxml2-dev locales locate lsb-release lsof make \
    net-tools netcat ninja-build pkg-config protobuf-compiler rsync rsyslog \
    ssh sysstat systemd-sysv systemtap-sdt-dev tinc tmux traceroute \
    tzdata unattended-upgrades uuid-dev uuid-runtime wget whiptail xfsprogs libeigen3-dev libsnappy-dev && \
    apt-get clean autoclean && \
    apt-get autoremove -y && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

FROM build_env as depbuild

# Copy cmake structure, without source code so that we can build and cache
# dependencies
COPY ./cmake /anemomind/cmake
COPY ./CMakeLists.txt /anemomind/CMakeLists.txt
RUN mkdir /anemomind/src && echo > /anemomind/src/CMakeLists.txt

RUN mkdir -p /anemomind/build
WORKDIR /anemomind/build
RUN cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo /anemomind

# build prod dependencies only
RUN make -j$(nproc) gtest_ext ceres_ext mongo_ext adolc_ext poco_ext

#copy all the code related to C++ : use .dockerignore for unwanted files
RUN rm -rf /anemomind/src
COPY ./src /anemomind/src
RUN make rebuild_cache

FROM depbuild as basebuild

# compile and link prod binaries.
RUN make -j$(nproc) nautical_processBoatLogs logimport_summary \
    anemobox_logcat logimport_try_load nautical_catTargetSpeed

# to compile and execute unit tests, do:
# docker build --target=unit_tester .
FROM basebuild as unit_tester

WORKDIR /anemomind/build
RUN make -j$(nproc) && make -j$(nproc) test


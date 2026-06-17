FROM i386/debian:jessie

# Use archive.debian.org as jessie is EOL
RUN echo "deb [trusted=yes] http://archive.debian.org/debian jessie main" > /etc/apt/sources.list && \
    echo "deb [trusted=yes] http://archive.debian.org/debian-security jessie/updates main" >> /etc/apt/sources.list && \
    echo "Acquire::Check-Valid-Until false;" > /etc/apt/apt.conf.d/10no-check-valid-until

# Install dependencies required for building anemobox
RUN apt-get update && apt-get install -y --force-yes \
    build-essential \
    wget \
    libssl-dev \
    git \
    libpoco-dev \
    libboost-thread-dev \
    protobuf-compiler \
    libprotobuf-dev \
    libboost-iostreams-dev \
    libbluetooth-dev \
    libboost-system-dev \
    libboost-filesystem-dev \
    libeigen3-dev \
    python \
    rsync \
    && rm -rf /var/lib/apt/lists/*

# Download and compile CMake from sources (latest 3.x)
RUN wget --no-check-certificate -qO- https://github.com/Kitware/CMake/releases/download/v3.31.12/cmake-3.31.12.tar.gz | tar xzf - && \
    cd cmake-3.31.12 && \
    ./bootstrap --parallel=$(nproc) && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -rf cmake-3.31.12

# Install Node.js v0.12.4
RUN wget -qO- https://nodejs.org/dist/v0.12.4/node-v0.12.4-linux-x86.tar.gz | tar -xz -C /usr/local --strip-components=1

ENV HOME=/tmp

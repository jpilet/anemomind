# Node base image - for both runtime & builder image
ARG CPP_DOCKER_IMAGE=anemocpp:latest

FROM debian:stretch-slim as node-base


ENV NODE_VERSION 8.11.3

RUN buildDeps='xz-utils' \
    && ARCH= && dpkgArch="$(dpkg --print-architecture)" \
    && case "${dpkgArch##*-}" in \
      amd64) ARCH='x64';; \
      ppc64el) ARCH='ppc64le';; \
      s390x) ARCH='s390x';; \
      arm64) ARCH='arm64';; \
      armhf) ARCH='armv7l';; \
      i386) ARCH='x86';; \
      *) echo "unsupported architecture"; exit 1 ;; \
    esac \
    && set -ex \
    && apt-get update && apt-get install -y ca-certificates curl wget gnupg dirmngr curl tar xz-utils ca-certificates sudo git python bash vim less wget bzip2 gzip iproute2 $buildDeps --no-install-recommends \
    && rm -rf /var/lib/apt/lists/* \
    && for key in \
      94AE36675C464D64BAFA68DD7434390BDBE9B9C5 \
      FD3A5288F042B6850C66B31F09FE44734EB7990E \
      71DCFD284A79C3B38668286BC97EC7A07EDE3FC1 \
      DD8F2338BAE7501E3DD5AC78C273792F7D83545D \
      C4F0DFFF4E8C1A8236409D08E73BC641CC11F4C8 \
      B9AE9905FFD7803F25714661B63B535A4C206CA9 \
      77984A986EBC2AA786BC0F66B01FBB92821C587A \
      8FCCA13FEF1D0C2E91008E09770F7A9A5AE15600 \
      4ED778F539E3634C779C87C6D7062848A1AB005C \
      A48C2BEE680E841632CD4E44F07496B3EB3C1762 \
      B9E2F5981AA6E0CD28160D9FF13993A75599653C \
    ; do \
      gpg --batch --keyserver hkp://p80.pool.sks-keyservers.net:80 --recv-keys "$key" || \
      gpg --batch --keyserver hkp://ipv4.pool.sks-keyservers.net --recv-keys "$key" || \
      gpg --batch --keyserver hkp://pgp.mit.edu:80 --recv-keys "$key" ; \
    done \
    && curl -fsSLO --compressed "https://nodejs.org/dist/v$NODE_VERSION/node-v$NODE_VERSION-linux-$ARCH.tar.xz" \
    && curl -fsSLO --compressed "https://nodejs.org/dist/v$NODE_VERSION/SHASUMS256.txt.asc" \
    && gpg --batch --decrypt --output SHASUMS256.txt SHASUMS256.txt.asc \
    && grep " node-v$NODE_VERSION-linux-$ARCH.tar.xz\$" SHASUMS256.txt | sha256sum -c - \
    && tar -xJf "node-v$NODE_VERSION-linux-$ARCH.tar.xz" -C /usr/local --strip-components=1 --no-same-owner \
    && rm "node-v$NODE_VERSION-linux-$ARCH.tar.xz" SHASUMS256.txt.asc SHASUMS256.txt \
    && apt-get purge -y --auto-remove $buildDeps \
    && ln -s /usr/local/bin/node /usr/local/bin/nodejs


##
# 2. Web Builder
###

FROM node-base as webbuilder


WORKDIR /anemomind/nodemodules

COPY nodemodules .

WORKDIR /anemomind/www2

COPY www2/package*.json ./

RUN  npm install --unsafe-perm=true --allow-root ../nodemodules/endpoint/

RUN  npm install --unsafe-perm=true --allow-root

COPY www2/. .

RUN npm install --unsafe-perm=true --allow-root -g mocha && \
    npm install --unsafe-perm=true --allow-root -g bower && \
    npm install --unsafe-perm=true --allow-root -g grunt 

RUN bower install --allow-root

## Wait for solution
ADD https://github.com/ufoscout/docker-compose-wait/releases/download/2.5.0/wait /wait
RUN chmod +x /wait

# If dev mode then expose port & start node server
EXPOSE 9000

###
# 3. Staging image for CPP binaries - because docker does not support ARG/ENV variables in COPY --from 
###

# FROM ${CPP_DOCKER_IMAGE} as cppbinaries


###
# 4. Web Application Runtime
###


FROM node-base
WORKDIR /anemomind

COPY --from=webbuilder /anemomind .
#RUN echo "copying cpp binaries from ${CPP_DOCKER_IMAGE}"

# Final runtime image doesn't make any shared libs assumptions - just copy from the appropriate CPP image
#COPY --from=cppbinaries /anemomind/lib/* /anemomind/lib/
#COPY --from=cppbinaries /anemomind/bin/* /anemomind/bin/

ENV PATH="/anemomind/www2/node_modules/.bin:${PATH}" \
          LD_LIBRARY_PATH="/anemomind/lib/"

# configure boat logs 
RUN mkdir -p /anemomind/www2/uploads/anemologs

EXPOSE 9000

WORKDIR /anemomind/www2

CMD ["node","server/app.js"]

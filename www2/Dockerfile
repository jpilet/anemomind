ARG CPP_DOCKER_IMAGE=anemomind_anemocppserver:latest

FROM debian:stretch-slim as node-base
ARG MONGO_URL
ARG GCLOUD_PROJECT
ARG GCS_KEYFILE
ARG GCS_BUCKET
ARG PUBSUB_TOPIC_NAME
ARG USE_GS
ENV MONGO_URL ${MONGO_URL}
ENV GCLOUD_PROJECT ${GCLOUD_PROJECT}
ENV GCS_KEYFILE ${GCS_KEYFILE}
ENV GCS_BUCKET ${GCS_BUCKET}
ENV PUBSUB_TOPIC_NAME ${PUBSUB_TOPIC_NAME}

RUN apt-get update && \
    apt-get install --no-install-recommends -y curl tar xz-utils ca-certificates sudo \
    git python bash less wget bzip2 gzip iproute2 && \
    apt-get clean && \
    apt-get autoclean

RUN curl -O https://nodejs.org/download/release/v8.11.3/node-v8.11.3-linux-x64.tar.xz 

RUN /bin/bash && \
    mkdir -p /anemomind/build/src/server/nautical/logimport/ && \
    mkdir -p /usr/local/lib/nodejs && \
    tar -xJvf node-v8.11.3-linux-x64.tar.xz -C /usr/local/lib/nodejs && \
    rm -rf node-v8.11.3-linux-x64.tar.xz && \
    export PATH=/usr/local/lib/nodejs/node-v8.11.3-linux-x64/bin:$PATH && \
    . ~/.profile

ENV PATH=/usr/local/lib/nodejs/node-v8.11.3-linux-x64/bin:$PATH \
    LD_LIBRARY_PATH="/anemomind/lib/"

WORKDIR /anemomind/nodemodules

COPY nodemodules .

WORKDIR /anemomind/www2

COPY www2/package*.json ./
COPY www2/anemomind-9b757e3fbacb.json ./
RUN  npm install --unsafe-perm=true --allow-root ../nodemodules/endpoint/ && \
    npm install --unsafe-perm=true --allow-root

COPY www2/. .

RUN npm install --unsafe-perm=true --allow-root -g mocha && \
    npm install --unsafe-perm=true --allow-root -g bower && \
    npm install --unsafe-perm=true --allow-root -g grunt 

RUN bower install --allow-root

## Wait for solution
ADD https://github.com/ufoscout/docker-compose-wait/releases/download/2.5.0/wait /wait
RUN chmod +x /wait && \
    mkdir -p uploads


###
# 3. Staging image for CPP binaries - because docker does not support ARG/ENV variables in COPY --from 
###

FROM ${CPP_DOCKER_IMAGE} as cppbinaries


###
# 4. Web Application Runtime
###


FROM node-base

WORKDIR /anemomind
RUN echo "copying cpp binaries from ${CPP_DOCKER_IMAGE}"

# Final runtime image doesn't make any shared libs assumptions - just copy from the appropriate CPP image
COPY --from=cppbinaries /anemomind/lib/* /anemomind/lib/
COPY --from=cppbinaries /anemomind/bin/* /anemomind/bin/

WORKDIR /anemomind/www2

CMD ["node","server/app.js"]

EXPOSE 9000


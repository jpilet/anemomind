ARG CPP_DOCKER_IMAGE=anemomind_anemocppserver:latest

#
# Image used for building only
#
FROM debian:bullseye-slim as node-base

RUN apt-get update && \
    apt-get install --no-install-recommends -y curl tar xz-utils ca-certificates sudo \
    git bash less wget bzip2 gzip iproute2 rsync ssh graphicsmagick graphicsmagick-imagemagick-compat && \
    apt-get clean && \
    apt-get autoclean

RUN curl -O https://nodejs.org/download/release/v8.11.3/node-v8.11.3-linux-x64.tar.xz 
RUN /bin/bash && \
    mkdir -p /usr/local/lib/nodejs && \
    tar -xJvf node-v8.11.3-linux-x64.tar.xz -C /usr/local/lib/nodejs && \
    rm -rf node-v8.11.3-linux-x64.tar.xz

ENV PATH="/usr/local/lib/nodejs/node-v8.11.3-linux-x64/bin:${PATH}"

FROM node-base as grunt-build

RUN npm install --unsafe-perm=true --allow-root -g mocha && \
    npm install --unsafe-perm=true --allow-root -g bower && \
    npm install --unsafe-perm=true --allow-root -g grunt grunt-cli

COPY www2/package*.json /anemomind/www2/
COPY nodemodules /anemomind/nodemodules/

WORKDIR /anemomind/www2
RUN  npm install --unsafe-perm=true --allow-root ../nodemodules/endpoint/ && \
     npm install --unsafe-perm=true --allow-root

# client or esa
ARG VHOST=client
COPY www2 /anemomind/www2/
RUN /anemomind/www2/build.sh

FROM grunt-build as web-dev

# Final runtime image doesn't make any shared libs assumptions - just copy from the appropriate CPP image
COPY bin/logimport_try_load bin/nautical_catTargetSpeed /anemomind/bin/
COPY lib /anemomind/lib/

# Those file are present on the image already and cause code dumps
# if present.
RUN cd /anemomind/lib && rm -f \
        ld-linux-x86-64.so.2 libc.so.6 libdl.so.2 libgcc_s.so.1 \
        libm.so.6 libpthread.so.0 libstdc++.so.6 linux-vdso.so.1
ENV LD_LIBRARY_PATH="/anemomind/lib/:${LD_LIBRARY_PATH}"
RUN ldconfig

CMD grunt serve:dev

#
# Image used for production
#

FROM node-base as www2

COPY --from=grunt-build /anemomind/www2/dist /app
COPY www2/utilities /app/utilities
COPY src/server/production/sendBoatDat.sh /anemomind/bin/

# Final runtime image doesn't make any shared libs assumptions - just copy from the appropriate CPP image
COPY bin/logimport_try_load bin/nautical_catTargetSpeed /anemomind/bin/
COPY lib /anemomind/lib/

# Those file are present on the image already and cause code dumps
# if present.
RUN cd /anemomind/lib && rm -f \
        ld-linux-x86-64.so.2 libc.so.6 libdl.so.2 libgcc_s.so.1 \
        libm.so.6 libpthread.so.0 libstdc++.so.6 linux-vdso.so.1
ENV LD_LIBRARY_PATH="/anemomind/lib/:${LD_LIBRARY_PATH}"
RUN ldconfig

## Wait for solution
ADD https://github.com/ufoscout/docker-compose-wait/releases/download/2.7.3/wait /wait
RUN /bin/chmod -v a+x /wait

ENV PATH=${PATH}:/usr/bin:/anemomind/bin \
  NODE_ENV=production \
  PORT=80

ARG MONGO_URL
ARG GCLOUD_PROJECT
ARG GCS_KEYFILE
ARG GCS_BUCKET
ARG PUBSUB_TOPIC_NAME
ARG USE_GS
ENV MONGO_URL=${MONGO_URL} \
  GCLOUD_PROJECT=${GCLOUD_PROJECT} \
  GCS_KEYFILE=${GCS_KEYFILE} \
  GCS_BUCKET=${GCS_BUCKET} \
  PUBSUB_TOPIC_NAME=${PUBSUB_TOPIC_NAME}

# Those have to be set when running the container
# ENV JWT_SECRET REPLACE_WITH_SECRET_HERE 
# ENV MONGOLAB_URI mongodb://anemomindprod:XXX@anemolab1,anemolab2,compute3/anemomind?replicaSet=rs0
# ENV SMTP_PASSWORD password

EXPOSE 80

CMD /wait && mkdir -p /db/uploads && node /app/server/app.js


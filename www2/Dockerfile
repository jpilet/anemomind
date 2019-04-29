FROM debian:9.8-slim

ARG MONGO_URL
ENV MONGO_URL ${MONGO_URL}

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

RUN  npm install --unsafe-perm=true --allow-root ../nodemodules/endpoint/

RUN  npm install --unsafe-perm=true --allow-root

COPY www2/. .

RUN npm install --unsafe-perm=true --allow-root -g mocha && \
    npm install --unsafe-perm=true --allow-root -g bower && \
    npm install --unsafe-perm=true --allow-root -g grunt 

RUN bower install --allow-root

## Wait for solution
ADD https://github.com/ufoscout/docker-compose-wait/releases/download/2.5.0/wait /wait

RUN chmod +x /wait && \
mkdir -p uploads

EXPOSE 9000


# CMD [ "npm", "start" ]

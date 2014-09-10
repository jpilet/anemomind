'use strict';

module.exports = {
  env: 'development',
  port: process.env.PORT || 8080,
  logfile: 'nodejs_server_dev.log',
  mongo: {
    uri: 'mongodb://localhost/anemomind-dev'
  }
};

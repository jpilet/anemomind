'use strict';

module.exports = {
  env: 'development',
  port: process.env.PORT || 80,
  logfile: 'nodejs_server.log',
  mongo: {
    uri: 'mongodb://localhost/fullstack-dev'
  }
};

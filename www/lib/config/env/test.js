'use strict';

module.exports = {
  env: 'test',
  port: process.env.PORT || 8050,
  logfile: 'nodejs_server_test.log',
  mongo: {
    uri: 'mongodb://localhost/fullstack-test'
  }
};

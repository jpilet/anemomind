'use strict';

module.exports = {
  env: 'production',
  port: process.env.PORT || 8080,
  logfile: '/srv/data/var/log/www/nodejs_server.log',
  mongo: {
    uri: process.env.MONGOLAB_URI ||
         process.env.MONGOHQ_URL ||
         'mongodb://localhost/fullstack'
  }
};

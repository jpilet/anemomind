'use strict';

module.exports = {
  env: 'production',
  port: process.env.PORT || 80,
  logfile: __dirname + '/../../../nodejs_server_prod.log',
  mongo: {
    uri: process.env.MONGOLAB_URI ||
         process.env.MONGOHQ_URL ||
         'mongodb://localhost/fullstack'
  }
};

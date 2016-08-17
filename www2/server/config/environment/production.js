'use strict';

// Production specific configuration
// =================================
module.exports = {
  // Server IP
  ip:       process.env.OPENSHIFT_NODEJS_IP ||
            process.env.IP ||
            undefined,

  // Server port
  port:     process.env.OPENSHIFT_NODEJS_PORT ||
            process.env.PORT ||
            80,

  // MongoDB connection options
  mongo: {
    uri:    process.env.MONGOLAB_URI ||
            process.env.MONGOHQ_URL ||
            process.env.OPENSHIFT_MONGODB_DB_URL+process.env.OPENSHIFT_APP_NAME ||
            'mongodb://localhost/anemomind'
  },

  uploadDir: '/var/uploads',
  endpointDir: '/var/mail2',
  backupDestination: 'anemomind@vtiger.anemomind.com:userlogs',

  ssl_OFF: {
    key: '/etc/ssl/anemolab.com/anemolab.com.key',
    cert: '/etc/ssl/anemolab.com/anemolab.com.cert',
    ip: '0.0.0.0',
    port: 443
  }
};

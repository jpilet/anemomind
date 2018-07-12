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

  uploadDir: '/db/uploads',
  endpointDir: '/db/mail2',
  backupDestination: 'anemomind@vtiger.anemomind.com:userlogs',

  tryLoadBin: "/home/jpilet/bin/logimport_try_load"
};

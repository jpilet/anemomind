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
  backupDestination: process.env.RSYNC_BACKUP_DESTINATION, 

  tryLoadBin: "logimport_try_load",

  // environment for GCP
  stripeSecretKey: process.env.STRIPE_SECRET_KEY || null,
  projectName : process.env.GCLOUD_PROJECT || "anemomind", 
  keyFile : process.env.GCS_KEYFILE || "/anemomind/www2/anemomind-9b757e3fbacb.json", 
  bucket : process.env.GCS_BUCKET || "boat_logs", 
  pubSubTopicName: process.env.PUBSUB_TOPIC_NAME || "anemomind_log_topic",
  useGoogleStorage : process.env.USE_GS || false

};

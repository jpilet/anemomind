'use strict';

// Development specific configuration
// ==================================
module.exports = {
  // MongoDB connection options
  mongo: {
    uri: process.env.MONGO_URL ||
         'mongodb://localhost/anemomind-dev'
  },

  backupWithRsync: false, 
  endpointDir:'/tmp/endpoints',

  tryLoadBin: process.env.LOAD_BIN_PATH || '/anemomind/bin/logimport_try_load',
  //Stripe secret key test mode
  stripeSecretKey: process.env.STRIPE_SECRET_KEY || null,
  // projectName : process.env.PROJECT_NAME || "anemomind", 
  // keyFile : process.env.KEYFILE || "/anemomind/www2/anemomind-9b757e3fbacb.json", 
  // bucket : process.env.BUCKET || "boat_logs", 
  // pubSubTopicName: process.env.PUBSUB_TOPIC_NAME || "anemomind_log_topic",
  projectName : process.env.GCLOUD_PROJECT || "anemomind", 
  keyFile : process.env.GCS_KEYFILE || "/anemomind/www2/anemomind-9b757e3fbacb.json", 
  bucket : process.env.GCS_BUCKET || "boat_logs", 
  pubSubTopicName: process.env.PUBSUB_TOPIC_NAME || "anemomind_log_topic",
  useGoogleStorage : false
};



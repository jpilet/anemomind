'use strict';

// Development specific configuration
// ==================================
module.exports = {
  // MongoDB connection options
  mongo: {  
      uri: process.env.MONGO_URL || 'mongodb://localhost/anemomind-dev'
  },

  endpointDir:'/tmp/endpoints',

  //Stripe secret key test mode
  stripeSecretKey: process.env.STRIPE_SECRET_KEY || null
};

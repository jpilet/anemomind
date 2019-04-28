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
  stripeSecretKey: 'sk_test_7m1Svu34PK6d75QUKvcBoHYC00Trdg2J0g',

  //pricing tab
  showPricingTab : true
};

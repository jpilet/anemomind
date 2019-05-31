
'use strict';

var mongoose = require('mongoose');
var Schema = mongoose.Schema;

var BillingHistorySchema = new Schema({
  subscriptionId: String, // subscriptionId + plan
  plan: String,
  susbcriptionStatus: String,
  subscriptionOwner: Schema.ObjectId,
  invoiceId: String,
  stripeUserId: String,
  date: Date,
  amount: Number,
  currency: String,
  hostedInvoiceLink: String
});

module.exports = mongoose.model('BillingHistory', BillingHistorySchema);
'use strict';

var mongoose = require('mongoose');
var Schema = mongoose.Schema;

var BillingSchema = new Schema({
  subscriptionId: String,
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

module.exports = mongoose.model('Billing', BillingSchema);
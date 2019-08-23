
'use strict';

const mongoose = require('mongoose');
const Schema = mongoose.Schema;

const BillingHistorySchema = new Schema({
  subscriptionId: String, // subscriptionId + plan
  plan: String,
  susbcriptionStatus: String,
  subscriptionOwner: Schema.ObjectId,
  invoiceId: String,
  stripeUserId: String,
  date: Date,
  amount: Number,
  currency: String,
  hostedInvoiceLink: String,
  boaatId: Schema.ObjectId
});

module.exports = mongoose.model('BillingHistory', BillingHistorySchema);
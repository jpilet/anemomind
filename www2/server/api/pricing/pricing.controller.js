'use strict';
// Ensure NODE_ENV is defined.
process.env.NODE_ENV = process.env.NODE_ENV || 'development';
var env = require('../../config/environment');
const stripe = require("stripe")(env.stripeSecretKey);

// Get list of plans
exports.getAllPlans = function (req, res) {
    stripe.plans.list(function (err, plans) {
        if (plans) {
            res.status(200).json(plans.data);
            console.log("Printing the plans now");
            console.log(plans.data);
        } else {
            res.status(400);
        }
    });
};

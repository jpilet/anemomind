'use strict';
// Ensure NODE_ENV is defined.
process.env.NODE_ENV = process.env.NODE_ENV || 'development';
var env = require('../../config/environment');
const stripe = require("stripe")(env.stripeSecretKey);

// this will be the cache obect o
var cachedSubscriptionPlans = [];
var basePlans = [];
var addOns = [];

function isEmptyObject(obj) {
    return !Object.keys(obj).length;
}

//Filter the base plan and addons from stripe
function segregatePlans(plans) {
    plans.forEach(element => {
        if (!!element.metadata.availableAddOns) {
            element.addOns = [];
            basePlans.push(element);
        }
        else {
            addOns.push(element);
        }
    });
    console.log(basePlans.length);
}

//create base subscription plan with addons to iterate over the template
function createSubscriptionPlans(baseplans, addOns) {
    addOns.forEach(addOn => {
        baseplans.forEach(basePlan => {
            if (basePlan.metadata.availableAddOns.includes(addOn.id)) {
                basePlan.addOns.push(addOn);
            }
        });
    });
    basePlans.sort(function (a, b) {
        return a.amount - b.amount;
    });
    console.log(basePlans.length);
}


// Get list of plans
exports.getAllPlans = function (req, res) {
    if (isEmptyObject(cachedSubscriptionPlans)) {
        stripe.plans.list(function (err, plans) {
            if (plans) {
                segregatePlans(plans.data);
                createSubscriptionPlans(basePlans, addOns);
                cachedSubscriptionPlans = basePlans;
                res.status(200).json(plans.data);
            } else {
                res.status(400);
            }
        });
    }
    else {
        res.status(200).json(cachedSubscriptionPlans);
    }
};

// Clear the cached plans 
exports.clearPlans = function (req, res) {
    cachedSubscriptionPlans = {};
    res.status(200).json({});
};

// To display the tab or not
exports.showPricingTab = function (req, res) {
    res.status(200).json({ 'showTab': env.showPricingTab });
};

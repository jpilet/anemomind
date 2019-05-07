'use strict';
// Ensure NODE_ENV is defined.
process.env.NODE_ENV = process.env.NODE_ENV || 'development';
const env = require('../../config/environment');

// To check if the stripe key is present or not
const isStripeKeyPresent = env.stripeSecretKey ? true : false;
const stripe = require("stripe")(env.stripeSecretKey);

// this will be the cache obect of subscription
const cachedSubscriptionPlans = {};

// Check if the object is empty or not
function isEmptyObject(obj) {
    return Object.keys(obj).length;
}

//Filter the base plan and addons from stripe
function segregatePlans(plans) {
    const basePlans = [];
    const addOns = [];
    plans.forEach(element => {
        if (!!element.metadata.availableAddOns) {
            element.addOns = [];
            basePlans.push(element);
        }
        else {
            addOns.push(element);
        }
    });
    plans = createSubscriptionPlans(basePlans, addOns);
    return plans;
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
    // Sorting the base plans based on the amount
    baseplans.sort(function (a, b) {
        return a.amount - b.amount;
    });
    cachedSubscriptionPlans.basePlans = baseplans;
    cachedSubscriptionPlans.addOns = addOns;
    return cachedSubscriptionPlans;
}


// Get list of plans
exports.getAllPlans = function (req, res) {
    if (isStripeKeyPresent) {
        if (!isEmptyObject(cachedSubscriptionPlans)) {
            stripe.plans.list(function (err, plans) {
                if (plans) {
                    const subscrptions = segregatePlans(plans.data);
                    res.status(200).json(subscrptions);
                } else {
                    res.status(400);
                }
            });
        }
        else {
            res.status(200).json(cachedSubscriptionPlans);
        }
    }
    else {
        res.status(500).json({ error: "Stripe-key unavailable" });
    }
};

// Clear the cached plans 
exports.clearPlans = function (req, res) {
    delete cachedSubscriptionPlans.basePlans;
    delete cachedSubscriptionPlans.addOns;
    res.status(200).json(cachedSubscriptionPlans);
};

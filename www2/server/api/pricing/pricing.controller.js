'use strict';
// Ensure NODE_ENV is defined.
process.env.NODE_ENV = process.env.NODE_ENV || 'development';
const env = require('../../config/environment');
// Used for Db connections
const mongoose = require('mongoose');
// get user schema to upadte object  
const User = require('./../user/user.model');
// get boat schema to upadte object
const boat = require('./../boat/boat.model');
// update the billingHistory model after listening from webhook
const Billing = require('./billingHistory.model');
// get the list of countries
const { getData } = require('country-list');

// Subscription status enum
const status = require('./subscriptionStatusEnum')

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


// Create a subscription for new user.
exports.createSubscription = function (req, res) {
    console.log("creating the customer now");
    let user = req.user;
    let details = req.body;
    // check if the customer has a stripe account or not already
    if (!!user.stripeUserId) {
        subscribetoPlan(user.stripeUserId, req.body.plan, res, req, user, req.body.boatId);
    }
    else {
        createStripeUser(req.body.plan, req.body.stripeSource, req.body.email, res, req, user, req.body.boatId);
    }
}

// list of countries.
exports.getCountries = function (req, res) {
    res.status(200).json(getData());
}

async function createStripeUser(plan, sourceStripeToken, email, res, req, user, boatId) {
    stripe.customers.create(
        {
            description: "Creating customer with card details" + email.toString(),
            email: email
        },
        function (err, customer) {
            if (customer) {
                console.log("Customer created successfully !! ");
                //Function to append the stripe Customer id to boat
                const customerId = customer.id;
                //once the customer is created then create the source 
                return createSourceCard(sourceStripeToken, customerId, plan, res, req, user, boatId);
            }
            else { res.status(500).json(err); }
        }
    );
}


async function createSourceCard(sourceStripeToken, customerId, plan, res, req, user, boatId) {
    console.log("Creating the card object for user");
    stripe.customers.createSource(
        customerId,
        { source: sourceStripeToken },
        function (err, card) {
            if (card) {
                console.log("card object created successully");
                return subscribetoPlan(customerId, plan, res, req, user, boatId);
            }
            else { res.status(500).json(err); }
        }
    );
}

async function subscribetoPlan(customerId, plan, res, req, user, boatId) {
    console.log("Subscribe the user to a plan");
    stripe.subscriptions.create(
        {
            customer: customerId,
            items: [
                { plan: plan }
            ],
            expand: ['latest_invoice.payment_intent']
        },
        function (err, subscription) {
            if (subscription) {
                console.log("Customer subscribed successfully !!");

                return updateUser(subscription, res, req, boatId);
            }
            else { res.status(500).json(err); }
        }
    );
}


async function updateUser(subscription, res, req, boatId) {
    try {
        User.findById(req.user._id, function (err, user) {
            if (err) {
                console.log("User not found");
                console.log(err);
                return res.status(404).json({
                    message: 'User not Found',
                    err: err
                });
            }
            else if (user) {
                user.stripeUserId = subscription.customer;
                user.save(function (err) {
                    if (err) {
                        console.log(err);
                        return res.status(400).json({
                            message: 'Error while updating user details',
                            err: err
                        });
                    }
                    console.log("User details update succefully " + user.name);
                    return updateBoat(subscription, res, boatId, user);
                });
            }
        });
    }
    catch (e) {
        console.log(e)
    }
}

async function updateBoat(subscription, res, boatId, user) {
    boat.findById(boatId, function (err, boat) {
        if (err) {
            console.log("Boat not found");
            console.log(err);
            return res.status(404).json({
                message: 'Boat not Found',
                err: err
            });
        }
        else if (boat) {
            boat.stripeUserId = subscription.customer;
            boat.subscriptionId = subscription.id;
            boat.plan = subscription.items.data[0].plan.id;
            boat.susbcriptionStatus = status.statusEnum.getValue("OPEN");
            boat.subscriptionOwner = user._id;
            boat.save(function (err) {
                if (err) {
                    console.log(err);
                    return res.status(400).json({
                        message: 'Error while updating boat details',
                        err: err
                    });
                }
                console.log("Boat details update succefully " + boatId);
                return res.status(200).json(subscription);
            });
        }
    });
}
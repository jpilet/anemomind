'use strict';
// Ensure NODE_ENV is defined.
process.env.NODE_ENV = process.env.NODE_ENV || 'development';
const env = require('../../config/environment');
// Used for Db connections
const mongoose = require('mongoose');
// get user schema to upadte object  
const User = require('./../user/user.model');
// get boat schema to upadte object
const Boat = require('./../boat/boat.model');
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
exports.createSubscription = async function (req, res) {
    console.log("creating the customer now");
    let user = req.user;
    let details = req.body;
    // check if the customer has a stripe account or not already
    if (!!user.stripeUserId) {
        subscribetoPlan(user.stripeUserId, req.body.plan, res, req, user, req.body.boatId);
    }
    else {
        let customer = await createStripeUser(req.body.email);
        // check if customer created successfully or not 
        if (!customer.id) {
            return res.status(500).json({ "message": "Error during creating customer", "error": result });
        }

        // create card object now - need this if the user updates the plan, need to charge him immidiately
        let sourceCard = await createSourceCard(customer.id, req.body.plan);
        if (!sourceCard.id) {
            return res.status(500).json({ "message": "Error during creating source", "error": sourceCard });
        }

        // now make the customer subscribe to plan/s based on selection on UI
        let subscription = await subscribetoPlan(customer.id, req.body.plan);
        if (!subscription.id) {
            return res.status(500).json({ "message": "Error during subscribing to plan", "error": subscription });
        }

        // make call to update user.
        let savedUser = await updateUser(subscription, req);
        if (!savedUser._id) {
            return res.status(500).json({ "message": "Error during updating user details", "error": user });
        }

        // make call to update customer.
        let boat = await updateBoat(subscription, req.boatId, savedUser);
        if (!boat._id) {
            return res.status(500).json({ "message": "Error during updating boat details", "error": boat });
        }

        // updating the details of the user
        return res.status(200).json(subscription);
    }
}

// list of countries.
exports.getCountries = function (req, res) {
    res.status(200).json(getData());
}

function createStripeUser(email) {
    return new Promise((resolve, reject) => {
        stripe.customers.create(
            {
                description: "Creating customer with card details" + email.toString(),
                email: email
            },
            function (err, customer) {
                if (customer) {
                    console.log("Customer created successfully !! ");
                    resolve(customer);
                }
                else {
                    console.log(err);
                    reject(err);
                }
            }
        );
    });
}


function createSourceCard(sourceStripeToken, customerId) {
    console.log("Creating the card object for user");
    return new Promise((resolve, reject) => {
        stripe.customers.createSource(
            customerId,
            { source: sourceStripeToken },
            function (err, card) {
                if (card) {
                    console.log("card object created successully");
                    resolve(card);
                }
                else {
                    console.log(err);
                    reject(err);
                }
            }
        );
    });
}

function subscribetoPlan(customerId, plan) {
    return new Promise((resolve, reject) => {
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
                    resolve(subscription);
                }
                else {
                    console.log(err);
                    reject(err);
                }
            }
        );
    });
}


function updateUser(subscription, req) {
    return new Promise((resolve, reject) => {
        try {
            let user = req.user;
            user.stripeUserId = subscription.customer;
            user.save(function (err) {
                if (err) {
                    console.log("Error while updating user details ", err);
                    reject(err);
                }
                console.log("User details update succefully ");
                resolve(user);
            });
        }
        catch (e) {
            console.log(e)
            reject(e);
        }
    });
}

function updateBoat(subscription, boatId, user) {
    Boat.findById(boatId, function (err, boat) {
        if (err) {
            console.log("Boat not Found", err);
            reject(err);
        }
        boat.stripeUserId = subscription.customer;
        boat.subscriptionId = subscription.id;
        // need to write a function to get the plans names from the suscription object
        boat.plan = subscription.items.data[0].plan.id;
        boat.susbcriptionStatus = status.statusEnum.OPEN;
        boat.subscriptionOwner = user._id;
        boat.save(function (err) {
            if (err) {
                console.log("Error while updating boat details ", err);
                reject(err);
            }
            console.log("Boat details update succefully");
            resolve(boat);
        });
    });
}
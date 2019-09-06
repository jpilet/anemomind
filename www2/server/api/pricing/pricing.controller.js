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

const planAbbreviations = [];

function checkIfPlanAdded(name) {
    // making use of for loop instead of foreach as we cannot break the look when 
    // we find the matched plan name
    for (var i = 0; i < planAbbreviations.length; i++) {
        if (planAbbreviations[i].planName === name) {
            return true;
        }
    }
}

//Filter the base plan and addons from stripe
function segregatePlans(plans) {
    const basePlans = [];
    const addOns = [];
    plans.forEach(element => {
        if (!!element.metadata.availableAddOns) {
            element.addOns = [];
            basePlans.push(element);
            // This is to insert the item at the very first time 
            // the plan abbreviations object will be empty and so any plan that is iterated at this 
            // will have to be added by default and as the first plan.
            if (planAbbreviations.length == 0) {
                planAbbreviations.push({
                    code: element.nickname.substring(0, 2).toLocaleUpperCase() + Math.floor(Math.random() * 10),
                    planName: element.nickname
                });
            }
            else {
                if (!!checkIfPlanAdded && !checkIfPlanAdded(element.nickname)) {
                    planAbbreviations.push({
                        code: element.nickname.substring(0, 2).toLocaleUpperCase() + Math.floor(Math.random() * 10),
                        planName: element.nickname
                    });
                }
            }
        }
        else {
            addOns.push(element);
            // the first element to be pushed in case plan abbreiviation is empty
            if (planAbbreviations.length == 0) {
                planAbbreviations.push({
                    code: element.nickname.substring(0, 2).toLocaleUpperCase() + Math.floor(Math.random() * 10),
                    planName: element.nickname
                });
            }
            else {
                if (!!checkIfPlanAdded && !checkIfPlanAdded(element.nickname)) {
                    planAbbreviations.push({
                        code: element.nickname.substring(0, 2).toLocaleUpperCase() + Math.floor(Math.random() * 10),
                        planName: element.nickname
                    });
                }
            }
        }
    });

    // Pushing the base plan with no value here.
    planAbbreviations.push({
        code: "DI" + + Math.floor(Math.random() * 10),
        planName: "Discovery"
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
    cachedSubscriptionPlans.planAbbreviations = planAbbreviations;
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
                    console.log(err);
                    res.status(400).json({ err: err });
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

function composePlans(plan) {
    let plans = plan.split(".");
    let selectedPlans = []
    plans.forEach(function (p) {
        planAbbreviations.forEach(function (abbr) {
            if (abbr.code === p) {
                if (abbr.code.substring(0, 2) !== "DI") {
                    selectedPlans.push({ plan: abbr.planName });
                }
            }
        });
    });
    console.log(selectedPlans);
    return selectedPlans;
}

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
        let sourceCard = await createSourceCard(req.body.stripeSource, customer.id);
        if (!sourceCard.id) {
            return res.status(500).json({ "message": "Error during creating source", "error": sourceCard });
        }

        console.log(req.body);
        let plan = composePlans(req.body.plan);
        // now make the customer subscribe to plan/s based on selection on UI
        let subscription = await subscribetoPlan(customer.id, plan);
        if (!subscription.id) {
            return res.status(500).json({ "message": "Error during subscribing to plan", "error": subscription });
        }

        // make call to update user.
        let savedUser = await updateUser(subscription, req);
        if (!savedUser._id) {
            return res.status(500).json({ "message": "Error during updating user details", "error": user });
        }

        // make call to update customer.
        let boat = await updateBoat(subscription, req.body.boatId, savedUser);
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
        console.log(plan);
        stripe.subscriptions.create(
            {
                customer: customerId,
                items: plan,
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

function updateBoat(subscription, boatId) {
    return new Promise((resolve, reject) => {
        Boat.findById(boatId, function (err, boat) {
            if (err) {
                console.log("Boat not Found", err);
                reject(err);
            }
            console.log(boat);
            boat.stripeUserId = subscription.customer;
            boat.subscriptionId = subscription.id;
            // Function that will get the plan names from the subscription object.
            boat.plan = getSubscribedPlanNames(subscription);
            boat.susbcriptionStatus = status.statusEnum.OPEN;
            boat.subscriptionOwner = subscription.customer;
            boat.save(function (err) {
                if (err) {
                    console.log("Error while updating boat details ", err);
                    reject(err);
                }
                console.log("Boat details update succefully");
                resolve(boat);
            });
        });
    });
}


function getSubscribedPlanNames(subscription) {
    return subscription.items.data.map(function (plan) { return plan.nickname; }).join('.');
}

// upgrade the existing subscription.
function updateSubscription(subscription) {
    return new Promise((resolve, reject) => {
        console.log("Upgrading the existing plan");
        stripe.subscriptionItems.update(
            'si_FTIVEvLBdElXQM', // subscriptionItemId -- to be updated
            //{ metadata: { order_id: '6735' } },
            function (err, subscriptionItem) {
                if (err) {
                    console.log(err);
                    reject(err);
                }
                else {
                    console.log("The subscription was updated successfully");
                    resolve(subscriptionItem);
                }
            }
        );
    });
}


// immidiately charge the customer to pay for the plan upgrade
function chargeOnSubscriptionUpdate(subscription) {
    return new Promise((resolve, reject) => {
        // asynchronously called
        stripe.invoices.create({
            // this will have the stripe customer id to charge him immidiately 
            // This is WIP .
            customer: "cus_FWUioTPJ6oSS08"
        }, function (err, invoice) {
            if (err) {
                reject(err);
            }
            else {
                resolve(invoice);
            }
        });
    });
}
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
    return planAbbreviations.map((x) => x.planName).indexOf(name) >= 0;
}

//Filter the base plan and addons from stripe
function segregatePlans(plans) {
    const basePlans = [];
    const addOns = [];
    plans.forEach(element => {
        if ('availableAddOns' in element.metadata) {
            element.addOns = [];
            basePlans.push(element);
            // This is to insert the item at the very first time 
            // the plan abbreviations object will be empty and so any plan that is iterated at this 
            // will have to be added by default and as the first plan.
            if (planAbbreviations.length == 0) {
                addAbbreviation(planAbbreviations, element)
            }
            else {
                if (!!checkIfPlanAdded && !checkIfPlanAdded(element.nickname)) {
                    addAbbreviation(planAbbreviations, element)
                }
            }
        }
        else {
            addOns.push(element);
            // the first element to be pushed in case plan abbreiviation is empty
            if (planAbbreviations.length == 0) {
                addAbbreviation(planAbbreviations, element)
            }
            else {
                if (!!checkIfPlanAdded && !checkIfPlanAdded(element.nickname)) {
                    addAbbreviation(planAbbreviations, element)
                }
            }
        }
    });

    // Pushing the base plan with no value here.
    planAbbreviations.push({
        code: "DI" + abbrConst,
        planName: "Discovery",
        price: 0
    });
    plans = createSubscriptionPlans(basePlans, addOns);
    return plans;
}


function addAbbreviation(planAbbreviations, element) {
    // Adding the abbreviations to the abbreviationsList page
    const code = element.nickname.substring(0, 2).toLocaleUpperCase();
    if (!planAbbreviations.map((x) => x.code).indexOf(code) >= 0) {
        planAbbreviations.push({
            code: code,
            planName: element.nickname,
            price: element.amount / 1000
        });
    }
    else {
        throw new Error("Duplicate abbreviation generated for plan :" + code + "-" + element.nickname);
    }
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
        subscribetoPlan(user.stripeUserId, req.body.plan, res, req, user, req.params.boatId);
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
        let boat = await updateBoat(subscription, req.params.boatId, savedUser);
        if (!boat._id) {
            return res.status(500).json({ "message": "Error during updating boat details", "error": boat });
        }

        // updating the details of the user
        return res.status(200).json(subscription);
    }
}


// update the subscription and update the users and boat details as well 
exports.updateSubscription = async function (req, res) {
    try {
        // make call to stripe to update the subscription.
        let subscription = await updateStripeSubscription(req.body.subId, req.body.plans)
        if (!subscription.id) {
            return res.status(500).json({ "message": "Error during subscribing to plan", "error": subscription });
        }

        // make call to update user.
        let savedUser = await updateUser(subscription, req);
        if (!savedUser._id) {
            return res.status(500).json({ "message": "Error during updating user details", "error": user });
        }

        // make call to update customer.
        let boat = await updateBoat(subscription, req.params.boatId, savedUser);
        if (!boat._id) {
            return res.status(500).json({ "message": "Error during updating boat details", "error": boat });
        }

        // updating the details of the user
        return res.status(200).json(subscription);
    }
    catch (ex) {
        return res.status(500).json(ex);
    }
}

// update the subscription and update the users and boat details as well 
exports.getProrationRates = async function (req, res) {
    //proration cost = (period end - API request time) / (period end - period start) * quantity * plan price
    let subscription = await getSubscriptionDetails(req.params.subId);
    if (!subscription.id) {
        console.log(subscrption);
        return res.status(400).json({ "message": "Invalid subscription id", "error": subscription });
    }
    else {
        // This will take the server time 
        let currentDate = new Date();
        // Doing this as the same can be used as proration date to consider pricing in invoice
        currentDate.setHours(0);
        currentDate.setMinutes(0);
        currentDate.setSeconds(0);

        const prorationDate = Math.floor(currentDate.getTime() / 1000);
        let prorationRates = []
        for (var i = 0; i < planAbbreviations.length; i++) {
            if (planAbbreviations[i].price > 0) {
                if ((subscription.current_period_end - subscription.current_period_end) > 0) {
                    let prorateCost = (subscription.current_period_end - prorationDate) / (subscription.current_period_end - subscription.current_period_end) * 1 * planAbbreviations[i].price;

                    // Add the proration rate for the plans in an array and return the same to the user
                    prorationRates.push({
                        price: prorateCost,
                        planName: planAbbreviations[i].planName
                    });
                }
            }
        }
        res.status(200).json(prorationRates);
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
function updateStripeSubscription(subId, plans) {
    return new Promise((resolve, reject) => {
        console.log("Upgrading the existing plan");
        stripe.subscriptions.update(
            subId,
            {
                items: plans,
            },
            function (err, subscription) {
                if (err) {
                    console.log("Error while updating subcription from stripe ", err);
                    reject(err);
                }
                else {
                    console.log("Stripe subscription updated successfully");
                    resolve(subscription)
                }
            }
        );
    });
}


// Get the details of the subscription by id
function getSubscriptionDetails(subId) {
    return new Promise((resolve, reject) => {
        stripe.subscriptions.retrieve(
            subId,
            function (err, subscription) {
                if (err) {
                    console.log("Error while getting subscription details from Stipe", err);
                    reject(err);
                }
                else {
                    resolve(subscription);
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

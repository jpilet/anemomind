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


// Create a subscription for new user.
exports.createSubscription = function (req, res) {
    console.log("creating the customer now");
    const subscription = createStripeUser(req.body.plan, req.body.stripeSource, req.body.email);
    res.status(200).json(subscription);
}


function createStripeUser(plan, sourceStripeToken, email) {
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
                return createSourceCard(sourceStripeToken, customerId, plan);
            }
            else { return err; }
        }
    );
}


function createSourceCard(sourceStripeToken, customerId, plan) {
    console.log("Creating the card object for user");
    stripe.customers.createSource(
        customerId,
        { source: sourceStripeToken },
        function (err, card) {
            if (card) {
                console.log("card object created successully");
                return subscribetoPlan(customerId, plan);
            }
            else { return err; }
        }
    );
}

function subscribetoPlan(customerId, plan) {
    console.log("Subscribe the user to a plan");
    stripe.subscriptions.create(
        {
            customer: customerId,
            items: [
                { plan: plan }
            ]
        },
        function (err, subscription) {
            if (subscription) {
                console.log("Customer subscribed successfully !!");
                return subscription;
            }
            else { return err; }

        }
    );
}

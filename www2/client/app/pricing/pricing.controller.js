angular.module('www2App')
  .controller('PricingCtrl', function ($scope, $http, Auth, boatList) {
    $scope.isLoggedIn = Auth.isLoggedIn;
    $scope.plans = [];
    $scope.plansLoaded = false;
    $scope.selectedBoat = "";
    $scope.boatNotSelected = false;
    $scope.isPlanSelected = false;

    // Get all the plans
    $http.get("/api/pricing/getAllPlans")
      .then(function (response) {
        $scope.plans = response.data.basePlans;
        $scope.plansLoaded = true;
      });

    // Get the list of boats for the current user
    // Will work only in case of the user is logged in
    if ($scope.isLoggedIn()) {
      boatList.boats().then(function (boats) {
        $scope.boats = boats;
        console.log(boats);
      });
    }

    // Need this just in case if the  user does not select any boat.
    $scope.changeBoat = function (boat) {
      $scope.selectedBoat = boat;
      $scope.boatNotSelected = false;
    }

    // Subscribe the user to a plans
    $scope.subscribe = function (id) {
      console.log($scope.selectedBoat);
      if (!!$scope.selectedBoat) {
        alert("Redirecting to the new page shortly")
      }
      else {
        $scope.boatNotSelected = true;
      }
      $scope.getUserDetails();
    }

    // Navigate the user to details page only if he is logged in 
    $scope.getUserDetails = function () {
      $scope.isPlanSelected = true;
    }

    // Take the user back to the plans page.
    $scope.cancel = function () {
      $scope.isPlanSelected = false;
    }


    // Create a Stripe client
    $scope.stripe = Stripe("pk_test_pBkRxSoJGZwe2JWkKmxVbz9M");

    // Create an instance of Elements
    $scope.elements = $scope.stripe.elements();

    // Custom styling can be passed to options when creating an Element.
    // (Note that this demo uses a wider set of styles than the guide below.)
    $scope.style = {
      base: {
        color: "#32325d",
        lineHeight: "24px",
        fontFamily: '"Helvetica Neue", Helvetica, sans-serif',
        fontSmoothing: "antialiased",
        fontSize: "16px",
        "::placeholder": {
          color: "#aab7c4"
        }
      },
      invalid: {
        color: "#fa755a",
        iconColor: "#fa755a"
      }
    };

    //Create an instance of the card number
    $scope.cardNumber = $scope.elements.create("cardNumber", { style: $scope.style });

    $scope.cardNumber.mount("#cardNumber");

    $scope.cardNumber.addEventListener("change", function (event) {
      var displayError = document.getElementById("cardNumber-errors");
      if (event.error) {
        displayError.textContent = event.error.message;
      } else {
        displayError.textContent = "";
      }
    });

    //Create an instance of the card Expiry
    $scope.cardExpiry = $scope.elements.create("cardExpiry", { style: $scope.style });

    $scope.cardExpiry.mount("#cardExpiry");

    $scope.cardExpiry.addEventListener("change", function (event) {
      var displayError = document.getElementById("cardExpiry-errors");
      if (event.error) {
        displayError.textContent = event.error.message;
      } else {
        displayError.textContent = "";
      }
    });

    //Create an instance of the card cvc
    $scope.cardCvc = $scope.elements.create("cardCvc", { style: $scope.style });

    $scope.cardCvc.mount("#cardCvc");

    cardCvc.addEventListener("change", function (event) {
      var displayError = document.getElementById("cardCvc-errors");
      if (event.error) {
        displayError.textContent = event.error.message;
      } else {
        displayError.textContent = "";
      }
    });

    //Create an instance of the postal code
    $scope.postalCode = $scope.elements.create("postalCode", { style: $scope.style });

    $scope.postalCode.mount("#postalCode");

    $scope.postalCode.addEventListener("change", function (event) {
      var displayError = document.getElementById("postalCode-errors");
      if (event.error) {
        displayError.textContent = event.error.message;
      } else {
        displayError.textContent = "";
      }
    });

    $scope.form = document.getElementById("payment-form-broken");
    var email = document.getElementById("email");
    $scope.ownerInfo = {
      owner: {
        name: "Jenny Rosen",
        address: {
          line1: "Nollendorfstraße 27",
          city: "Berlin",
          postal_code: "10777",
          country: "DE"
        }
      }
    };
    $scope.form.addEventListener("submit", function (event) {
      event.preventDefault();

      $scope.stripe.createSource($scope.cardNumber, $scope.ownerInfo).then(function (result) {
        if (result.error) {
          // Inform the user if there was an error
          var errorElement = document.getElementById("card-errors");
          errorElement.textContent = result.error.message;
        } else {
          // Send the source to your server
          stripeSourceHandler(result.source);
        }
      });
    });

    function stripeSourceHandler(source) {
      // Insert the token ID into the form so it gets submitted to the server
      var data = {};
      data.stripeSource = source.id;
      data.email = document.getElementById("email").value;
      data.country = document.getElementById("country").value;
      data.plan = "navigation_memories_base_plan_chf";

      // Submit the form
      $scope.subscribeUser(data);
    }

    $scope.subscribeUser = function (data) {
      $http.post("/api/pricing/subscribe", data)
        .then(function (response) {
          console.log(response);
        });
    }

  });

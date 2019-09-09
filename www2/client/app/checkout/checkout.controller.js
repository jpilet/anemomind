angular.module('www2App')
  .controller('CheckoutCtrl', function ($scope, $stateParams, $http, Auth, Checkout) {

    $scope.boat = Checkout.getBoat();
    $scope.boatId = $stateParams.boatId;
    $scope.plan = $stateParams.plan;

    $scope.isLoggedIn = Auth.isLoggedIn;
    if ($scope.isLoggedIn()) {
      var user = Auth.getCurrentUser();
      $scope.name = user.name;
      $scope.email = user.email;
    }

    // Model and country list
    $scope.countries = {
      selectedCountry: null,
      countries: []
    };

    // get the list of countries
    $http.get("/api/pricing/getCountries")
      .then(function (response) {
        $scope.countries.countries = response.data;
      });

    // Create a Stripe client
    $scope.stripe = Stripe("pk_test_pBkRxSoJGZwe2JWkKmxVbz9M");

    // Create an instance of Elements
    $scope.elements = $scope.stripe.elements();

    // Custom styling can be passed to options when creating an Element.
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

    $scope.cardCvc.addEventListener("change", function (event) {
      let displayError = document.getElementById("cardCvc-errors");
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

    $scope.form.addEventListener("submit", function (event) {
      event.preventDefault();
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

      $scope.stripe.createSource($scope.cardNumber, $scope.ownerInfo).then(function (result) {
        if (result.error) {
          // Inform the user if there was an error
          var errorElement = document.getElementById("card-errors");
          errorElement.textContent = result.error.message;
        } else {
          // Send the source to server
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
      data.plan = $scope.plan;
      data.boatId = $scope.boatId;
      // Submit the form
      $scope.subscribeUser(data);
    }

    $scope.subscribeUser = function (data) {
      $scope.processingRequest = true;
      $http.post("/api/pricing/subscribe", data)
        .then(function (response) {
          console.log(response);
          if (response.data.latest_invoice.payment_intent.status === 'requires_action') {
            $scope.stripe.handleCardPayment(response.data.latest_invoice.payment_intent.client_secret).then(function (result) {
              if (result.error) {
                $scope.processingRequest = true;
                alert(result.error)
              } else {
                $scope.processingRequest = true;
                alert("Success");
              }
            });
          }
          else if (response.data.latest_invoice.payment_intent.status === 'succeeded') {
            $scope.processingRequest = true;
            alert("Success");
          }
        });
    }
  });
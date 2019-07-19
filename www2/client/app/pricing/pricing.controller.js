angular.module('www2App')
  .controller('PricingCtrl', function ($scope, $http, $location, Auth, boatList, Checkout) {
    $scope.isLoggedIn = Auth.isLoggedIn;
    $scope.plans = [];
    $scope.selectedPlan = "";
    $scope.plansLoaded = false;
    $scope.plansAbbreviation = [];
    $scope.selectedBoat = {
      boatId: null,
      boats: []
    }
    $scope.boatNotSelected = false;
    $scope.isPlanSelected = false;

    // Get all the plans
    $http.get("/api/pricing/getAllPlans")
      .then(function (response) {
        $scope.plans = response.data.basePlans;
        $scope.plansAbbreviation = response.data.planAbbreiviations;
        $scope.plansLoaded = true;
      });

    // if the $location.$$search.boatId is undefined the model value will be set as null
    if (!!$location.search) {
      $scope.selectedBoat.boatId = $location.$$search.boatId;
    }

    // Get the list of boats for the current user
    boatList.boats().then(function (boats) {
      $scope.selectedBoat.boats = boats.filter(function (boat) {
        return boat.admins.indexOf(Auth.getCurrentUser() > -1);
      });
      // if the list has only one item, it should be preselected.
      if ($scope.selectedBoat.boats.length === 1) {
        $scope.selectedBoat.boatId = $scope.selectedBoat.boats[0]._id
      }
    });

    // Change the value in url on boat selection
    $scope.changeBoat = function () {
      // $location.search("boatId", $scope.selectedBoat.boatId);
      $scope.boatNotSelected = false;
      $scope.selectedBoat.boats.forEach(function (element) {
        if (element._id === $scope.selectedBoat.boatId) {
          $scope.boatName = element.name;
          // set the value of the selected boat
          Checkout.setBoat(element);
        }
      });
    }

    // Navigate the users to checkout page
    $scope.subscribe = function (plan) {
      let addons = plan.addOns;
      var selectedPlan = "";

      // Will populate plan and addons together if selected
      addons.forEach(function (element) {
        var isAddOnSelected = document.getElementById(plan.id + '-' + element.id).checked;
        if (isAddOnSelected) {
          $scope.plansAbbreviation.forEach(function (p) {
            if (p.planName === element.id) {
              if (selectedPlan === "") {
                selectedPlan = p.code;
              }
              else {
                selectedPlan = selectedPlan + "." + p.code;
              }
            }
            if (plan.id === p.planName) {
              if (selectedPlan === "") {
                selectedPlan = p.code
              }
              else {
                selectedPlan.
                selectedPlan = p.code + "." + selectedPlan
              }
            }
          });
        }
      });

      // Will populate the plan only if base plan selected
      if(selectedPlan === ""){
        $scope.plansAbbreviation.forEach(function (p) {
          if (plan.id === p.planName) {
            if (selectedPlan === "") {
              selectedPlan = p.code
            }
            else {
              selectedPlan = p.code + "." + selectedPlan
            }
          }
        });
      }

      if ($scope.selectedBoat.boatId !== 'undefined' && $scope.selectedBoat.boatId !== null) {
        $scope.isPlanSelected = true;
        // set the value of the selected plan
        Checkout.setSelectedPlan($scope.selectedPlan);
        $location.path('/checkout/' + $scope.selectedBoat.boatId).search({ plan: selectedPlan });
      }
      else {
        $scope.boatNotSelected = true;
      }
    }
  });

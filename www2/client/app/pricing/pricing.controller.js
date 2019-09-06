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
      var selectedPlan = [];

      // Will populate plan and addons together if selected
      addons.forEach(function (element) {
        // The convetion used to identify if the add on checked is
        // baseplan.id_addon.id so if we have multiple base plans with add ons
        // the one that will get seleted are those whose base plan is selected.
        var isAddOnSelected = document.getElementById(plan.id + '-' + element.id).checked;
        if (isAddOnSelected) {
          $scope.plansAbbreviation.forEach(function (p) {
            if (p.planName === element.id) {
              selectedPlan.push(p.code);
            }
          });
        }
      });

      // Will populate the plan only if base plan selected
      // This means that no add on was selected and the user has selected the base plan only
      if (selectedPlan.length === 0) {
        $scope.plansAbbreviation.forEach(function (p) {
          if (plan.id === p.planName) {
            selectedPlan.push(p.code);
          }
        });
      }

      // convert selectedPlan to a string
      var planStr = selectedPlan.join('.');

      if ($scope.selectedBoat.boatId !== 'undefined' && $scope.selectedBoat.boatId !== null) {
        $scope.isPlanSelected = true;
        // set the value of the selected plan
        Checkout.setSelectedPlan(planStr);
        $location.path('/checkout/' + $scope.selectedBoat.boatId).search({ plan: planStr });
      }
      else {
        $scope.boatNotSelected = true;
      }
    }
  });

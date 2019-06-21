angular.module('www2App')
  .controller('PricingCtrl', function ($scope, $http, $location, Auth, boatList, Checkout) {
    $scope.isLoggedIn = Auth.isLoggedIn;
    $scope.plans = [];
    $scope.selectedPlan = "";
    $scope.plansLoaded = false;
    $scope.selectedBoat = {
      model: null,
      boats: []
    }
    $scope.boatNotSelected = false;
    $scope.isPlanSelected = false;

    // Get all the plans
    $http.get("/api/pricing/getAllPlans")
      .then(function (response) {
        $scope.plans = response.data.basePlans;
        $scope.plansLoaded = true;
      });

    // if the $location.$$search.boatId is undefined the model value will be set as null
    if (!!$location.search) {
      $scope.selectedBoat.model = $location.$$search.boatId;
    }

    // Get the list of boats for the current user
    boatList.boats().then(function (boats) {
      $scope.selectedBoat.boats = boats.filter(function (boat) {
        return boat.admins.indexOf(Auth.getCurrentUser() > -1);
      });
      // if the list has only one item, it should be preselected.
      if ($scope.selectedBoat.boats.length === 1) {
        $scope.selectedBoat.model = $scope.selectedBoat.boats[0]._id
      }
    });

    // Change the value in url on boat selection
    $scope.changeBoat = function () {
      $location.search("boatId", $scope.selectedBoat.model);
      $scope.boatNotSelected = false;
      $scope.selectedBoat.boats.forEach(function (element) {
        if (element._id === $scope.selectedBoat.model) {
          $scope.boatName = element.name;
          // set the value of the selected boat
          Checkout.setBoat(element);
        }
      });
    }

    // Navigate the users to checkout page
    $scope.subscribe = function (id) {
      $scope.selectedPlan = id;
      if ($scope.selectedBoat.model !== 'undefined' && $scope.selectedBoat.model !== null) {
        $scope.isPlanSelected = true;
        // set the value of the selected plan
        Checkout.setSelectedPlan($scope.selectedPlan);
        $location.path('/checkout' + $scope.selectedBoat.model);
      }
      else {
        $scope.boatNotSelected = true;
      }
    }
  });

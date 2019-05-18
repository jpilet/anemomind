angular.module('www2App')
  .controller('PricingCtrl', function ($scope, $http, Auth, boatList) {
    $scope.isLoggedIn = Auth.isLoggedIn;
    $scope.plans = [];
    $scope.plansLoaded = false;
    $scope.selectedBoat = "";
    $scope.boatNotSelected = false;

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
    }

  });

angular.module('www2App')
  .controller('PricingCtrl', function ($scope, $http, Auth) {
    $scope.isLoggedIn = Auth.isLoggedIn();
    $scope.plans = [];
    $scope.plansLoaded = false;
    console.log("print if logged in or not: " + $scope.isLoggedIn);

    $http.get("/api/pricing/getAllPlans")
      .then(function (response) {
        $scope.plans = response.data.basePlans;
        $scope.plansLoaded = true;
      });
  });

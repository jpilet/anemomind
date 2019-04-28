angular.module('www2App')
  .controller('PricingCtrl', function ($scope, $http, Auth) {
    $scope.isLoggedIn = Auth.isLoggedIn();
    $scope.plans = [];
    $scope.plansLoaded = false;

    $http.get("/api/pricing/getAllPlans")
      .then(function (response) {
        $scope.plans = response.data;
        $scope.plansLoaded = true;
      });
  });

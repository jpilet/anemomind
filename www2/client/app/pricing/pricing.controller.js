angular.module('www2App')
  .controller('PricingCtrl', function ($scope, $http, Auth) {
    $scope.isLoggedIn = Auth.isLoggedIn();
  });


angular.module('www2App')
  .controller('AboutCtrl', function ($scope, $http, Auth) {
    $scope.isLoggedIn = Auth.isLoggedIn();
    $scope.loading = true;

    $scope.$watch(Auth.isLoggedIn, function(newVal, oldVal) {
      $scope.isLoggedIn = newVal;
    });
  });

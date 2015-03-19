'use strict';

angular.module('www2App')
 .controller('PathController', [ '$scope', 'Auth', function($scope, Auth) {
    $scope.isLoggedIn = Auth.isLoggedIn();
    $scope.$watch(Auth.isLoggedIn, function(newVal, oldVal) {
      if (newVal && newVal != oldVal) {
        $scope.isLoggedIn = newVal;
      }
    });
}]);

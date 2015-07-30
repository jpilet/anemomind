'use strict';

angular.module('www2App')
 .controller('PathController',
             function($scope, Auth, $http, boatList) {
    $scope.isLoggedIn = Auth.isLoggedIn();

    $scope.boats = boatList.boats();
    $scope.$on('boatList:updated', function(event, boats) {
       $scope.boats = boats;
    });

    $scope.$watch(Auth.isLoggedIn, function(newVal, oldVal) {
      $scope.isLoggedIn = newVal;
    });
});

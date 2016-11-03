'use strict';

angular.module('www2App')
  .controller('RewardsCtrl', function ($scope, Auth) {
    $scope.isAdmin = Auth.isAdmin;
    $scope.isLoggedIn = Auth.isLoggedIn();
  });

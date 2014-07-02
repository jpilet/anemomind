'use strict';

angular.module('anemomindApp')
  .controller('SettingsCtrl', function ($scope, User) {
    $scope.errors = {};
    $scope.user = User.get();

    $scope.changePreferences = function() {

      User.update($scope.user, function(result) {
        console.log(result);
        if (result['0'] === 'O' && result['1'] === 'K') {
          $scope.message = 'Your preferences have been changed successfully !';
        }
      });

      // $scope.message = User.update ($scope.user);
      // console.log($scope.message);
    };
  });
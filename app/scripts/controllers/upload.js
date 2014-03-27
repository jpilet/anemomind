'use strict';

angular.module('anemomindApp')
  .controller('UploadCtrl', function ($scope, User) {
    $scope.errors = {};
    $scope.user = User.get();

    $scope.upload = function() {

      console.log('should upload...');
    };
  });
'use strict';

angular.module('www2App')
  .controller('BoxexecCtrl', function ($scope, BoxExec) {
    $scope.boxexecs = BoxExec.query();
  });

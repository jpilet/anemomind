'use strict';

angular.module('anemomindApp')
  .controller('RaceListCtrl', function ($scope, Race, $http, $log) {
    $scope.races = Race.get();
  });

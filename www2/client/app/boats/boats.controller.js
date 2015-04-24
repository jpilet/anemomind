'use strict';

angular.module('www2App')
  .controller('BoatsCtrl', function ($scope, $http, socket, boatList) {

    $scope.boats = boatList.boats();
    $scope.$on('boatList:updated', function(event,data) {
      $scope.boats = data;
    });

    $scope.addBoat = function() {
      if($scope.newBoat === '') {
        return;
      }
      $http.post('/api/boats', { name: $scope.newBoat });
      $scope.newBoat = '';
      boatList.update();
    };
  });

'use strict';

angular.module('www2App')
  .controller('BoatsCtrl', function ($scope, $http, socket) {
  	$scope.boats = [];
    $http.get('/api/boats')
    .success(function(data, status, headers, config) {
       $scope.boats = data;
       socket.syncUpdates('boat', $scope.boats);
    });
    $scope.addBoat = function() {
      if($scope.newBoat === '') {
        return;
      }
      $http.post('/api/boats', { name: $scope.newBoat });
      $scope.newBoat = '';
    };

    $scope.deleteBoat = function(boat) {
      $http.delete('/api/boats/' + boat._id);
    };

    $scope.$on('$destroy', function () {
      socket.unsyncUpdates('boat');
    });
  });

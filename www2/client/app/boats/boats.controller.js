'use strict';

angular.module('www2App')
  .controller('BoatsCtrl', function ($scope, $http, $stateParams,$location, socket, boatList,Auth,$log,$timeout) {
    $scope.isLoggedIn = Auth.isLoggedIn();

    $scope.$watch(Auth.isLoggedIn, function(newVal, oldVal) {
      $scope.isLoggedIn = newVal;
    });

    
    if ($stateParams.boatId) {
        $scope.boatId=$stateParams.boatId;
    } else {
      boatList.boats().then(function(boats) {
        $scope.boats = boats;

        $scope.boatId = boatList.getDefaultBoat();
        if (!$scope.boatId) {
          // redirect to the create boat page
          $location.path('/boats');
        }
      });
    }

    $scope.addBoat = function() {
      if(!$scope.newBoat||$scope.newBoat === '') {
        return;
      }
      $http.post('/api/boats', { name: $scope.newBoat });
      $scope.newBoat = '';
      boatList.update();
    };


  });

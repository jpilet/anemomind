'use strict';

angular.module('www2App')
  .controller('BoatsCtrl', function ($scope, $http, $stateParams,$location, socket, boatList,Auth,$log,$timeout) {
    $scope.isLoggedIn = Auth.isLoggedIn();
    $scope.errorMessage = undefined;
    $scope.boatCreated = undefined;

    // For some reason, for the two-way binding to work properly between the
    // html and the controller, scope members have to be initialized with something
    // valid. So to initialize a variable to something non-empty but containing an
    // empty string, we make an object containing just one member.
    // Trick found on
    // http://stackoverflow.com/questions/12618342/ng-model-does-not-update-controller-value
    $scope.newBoat = { name: '' };

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
      $scope.errorMessage = undefined;

      var newBoatName = $scope.newBoat.name = ('' + $scope.newBoat.name).trim();
      if(!newBoatName || newBoatName === '') {
        $scope.errorMessage = 'Please enter a boat name';
        return;
      }

      $scope.newBoat.name = '';

      $http.post('/api/boats', { name: newBoatName })
        .then(function(boat) {
          boatList.update();
          $scope.boatCreated = boat.data;
        })
      .catch(function(err) {
        $scope.errorMessage = 'Failed to create boat.';
      });
    };


  });

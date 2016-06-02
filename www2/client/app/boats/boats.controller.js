'use strict';

angular.module('www2App')
  .controller('BoatsCtrl', function ($scope, $http, $stateParams, socket, boatList,Auth) {
    $scope.isLoggedIn = Auth.isLoggedIn();

    $scope.$watch(Auth.isLoggedIn, function(newVal, oldVal) {
      $scope.isLoggedIn = newVal;
    });

    

    //
    // TODO (OLIVIER) depending the user scenario, the boats can be empty
    // that should take care about Auth
    boatList.boats().then(function(boats) {
      $scope.boats = boats;


      //
      // display selected boat
      if($stateParams.boatId){
        $scope.boatId=$stateParams.boatId;
        return;
      }

      //
      // display default boat
      $scope.boatId=boatList.getDefaultBoat()._id;

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

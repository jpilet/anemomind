'use strict';

angular.module('www2App')
  .controller('BoatadminCtrl', function ($scope, Auth, boatList) {
    $scope.isAdmin = Auth.isAdmin;

    $scope.remoteCommand = '';

    $scope.boats = boatList.boats();
    $scope.$on('boatList:updated', function(event, boats) {
       $scope.boats = boats;
    });

    $scope.toggleAll = function() {
      for (var i in $scope.boats) {
        var boat = $scope.boats[i];
        boat.selected = !boat.selected;
      }
    };

    $scope.setSelectionAll = function(yesno) {
      for (var i in $scope.boats) {
        $scope.boats[i].selected = !!yesno;
      }
    };

    $scope.delete = function(boat) {
      // TODO
    };

    $scope.sendCommand = function() {
    };

  });

'use strict';

angular.module('www2App')
 .controller('PathController',
             function($scope, Auth, $http, boatList) {
    $scope.isLoggedIn = Auth.isLoggedIn();

    $scope.boats = boatList.boats();
    $scope.$on('boatList:updated', function(event, boats) {
       $scope.boats = boats;
    });

    $scope.$watch(Auth.isLoggedIn, function(newVal, oldVal) {
      $scope.isLoggedIn = newVal;
    });

    $scope.boatFreshness = function(boat) {
      var sessions = boatList.sessionsForBoat(boat._id);
      if (!sessions || sessions.length == 0) {
        return "0000";
      }
      var date = sessions[0].startTime;
      for (var i = 1; i < sessions.length; ++i) {
        if (sessions[i].startTime > date) {
          date = sessions[i].startTime;
        }
      }
      return date;
    };
});

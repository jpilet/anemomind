'use strict';

angular.module('www2App')
  .controller('RewardsCtrl', function ($scope, Auth) {
    $scope.isAdmin = Auth.isAdmin;
    $scope.isLoggedIn = Auth.isLoggedIn();

    $scope.maxLevel = 10;
    var maxBars = 5;
    $scope.currentLevel = 0;

    // The number of levels inside a bar
    // are actually 5 and not 3
    var displayLevels = function(currentLevel) {
      var max = currentLevel + 4;
      var min = max >= 11 ? 7 : currentLevel;

      // Instead of 10, we set it by 11
      // but we don't have to fill this level
      // with bars
      max = max >= 11 ? 11 : max;      

      return {min: min, max: max };
    };
    console.log(displayLevels(5));

    var fillUpWithBars = function(valueGiven, valueToReach) {
      var diff = valueToReach - valueGiven;
      var perc = diff / valueToReach;
      var bars = Math.round(maxBars - (maxBars * perc));

      return bars;
    };
    console.log(fillUpWithBars(15, 20));
    
  });

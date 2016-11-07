'use strict';

angular.module('www2App')
  .controller('RewardsCtrl', function ($scope, Auth) {
    $scope.isAdmin = Auth.isAdmin;
    $scope.isLoggedIn = Auth.isLoggedIn();

    var maxBars = 5;

    $scope.challenges = [
      {
        name: 'Hard workers',
        icon: 'fa-user',
        level: 3,
        progress: 153,
        goal: 160,
        unit: 'manuevers',
        desc: 'Number of manuevers',
        displayedLevels: null
      },
      {
        name: 'Round the world',
        icon: 'fa-map',
        level: 5,
        progress: 50,
        goal: 120,
        unit: 'km',
        desc: 'Distance reached',
        displayedLevels: null
      },
      {
        name: 'Fast and furious',
        icon: 'fa-dashboard',
        level: 7,
        progress: 6,
        goal: 10,
        unit: 'kts avg on a 3sec period',
        desc: 'Maximum speed reached',
        displayedLevels: null
      },
      {
        name: 'Photo-grapher',
        icon: 'fa-camera',
        level: 9,
        progress: 15,
        goal: 25,
        unit: 'pictures',
        desc: 'Numbers of pictures taken',
        displayedLevels: null
      },
      {
        name: 'Scribe',
        icon: 'fa-file',
        level: 10,
        progress: 16,
        goal: 16,
        unit: 'notes',
        desc: 'Number of notes',
        displayedLevels: null
      }
    ];

    // The goal is to display the levels by 3's 
    // The format based from the mockup will be like:
    // 0 [1 2 3]
    // 3 [4 5 6]
    // 6 [7 8 9]
    // 7 [8 9 10]
    var displayLevels = function(currentLevel) {
      var min, max = 0;

      if(currentLevel <= 3) {
        min = 1;
      }
      else if(currentLevel <= 6) {
        min = 4;
      }
      else if(currentLevel <= 9) {
        min = 7;
      }
      else if(currentLevel == 10) {
        min = 8;
      }
      max = min+2;

      return {min: min, max: max };
    };

    // So we only calculate if it's the current level
    // Less than the current level will have 5 bars
    // Greater than the current level will have 0 bars
    var fillUpWithBars = function(level, index) {
      if(index > level.level)
        return 0;

      if(index < level.level)
        return maxBars;

      var diff = level.goal - level.progress;
      var perc = diff / level.goal;
      var bars = Math.floor(maxBars - (maxBars * perc));

      return bars;
    };
    

    for(var i in $scope.challenges) {
      var array = [];
      var data = null;
      var levels = displayLevels($scope.challenges[i].level);
      for(var l = levels.min; l <= levels.max; l++) {
        data = {
          level: l,
          bars: fillUpWithBars($scope.challenges[i], l)
        };

        array.push(data);
      }
      $scope.challenges[i].displayedLevels = array;
    }


    // To display the count of bars
    // in ng-repeat
    $scope.range = function(count){
      var ratings = []; 

      for (var i = 0; i < count; i++) { 
        ratings.push(i) 
      }
      return ratings;
    }
  });

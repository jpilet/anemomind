'use strict';

angular.module('www2App')
  .controller('RewardsCtrl', function ($scope, Auth) {
    $scope.isAdmin = Auth.isAdmin;
    $scope.isLoggedIn = Auth.isLoggedIn();

    var maxBars = 5;

    $scope.rewards = [
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

    $scope.infos = [
      {
        name: 'Community',
        desc: 'Boat data sharing is enabled with others',
        active: false,
      },
      {
        name: 'Share',
        desc: 'Share at least one track on social media',
        active: true,
      },
      {
        name: 'Competitor',
        desc: 'Get ranked in all challenges',
        active: false,
      },
      {
        name: 'The Ultimate',
        desc: 'Get maximum ranking at each challenges excluding speed',
        active: false,
      }
    ];

    // To display the count of bars
    // in ng-repeat
    $scope.range = function(count){
      var ratings = []; 

      for (var i = 0; i < count; i++) { 
        ratings.push(i) 
      }
      return ratings;
    }

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
      else if(currentLevel >= 10) {
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
    

    for(var i in $scope.rewards) {
      var array = [];
      var data = null;
      var levels = displayLevels($scope.rewards[i].level);
      for(var l = levels.min - 1; l <= levels.max; l++) {
        data = {
          level: l,
          bars: fillUpWithBars($scope.rewards[i], l)
        };

        array.push(data);
      }
      $scope.rewards[i].displayedLevels = array;
    }


    $scope.popup = {
      content: '',
      templateUrl: 'descriptionTemplate.html',
      title: ''
    };
    $scope.fillDesc = function(desc) {
      $scope.itemDesc = desc;
    }

    // Only show the pop-ups on stated widths
    // or lesser of it
    var rewardThreshold = 767;
    var infoThreshold = 412;

    var container = angular.element(window);
    var width = function() {
      return container.width();
    };

    $scope.rewardsPopupActive = (width() <= rewardThreshold);
    $scope.infoPopupActive = (width() <= infoThreshold);
    
    container.on('resize', function() {
      $scope.rewardsPopupActive = (width() <= rewardThreshold);
      $scope.infoPopupActive = (width() <= infoThreshold);
    });


    // Responsive config for the Slick slider
    $scope.breakpoints = [
      {
        breakpoint: 768,
        settings: {
          slidesToShow: 1
        }
      }
    ];
  });

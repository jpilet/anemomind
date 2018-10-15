'use strict';

function perfAtPoint(d) {
    var field = 'devicePerf';
    if (field in d) {
      return d[field];
    } else if ('deviceVmg' in d
               && 'deviceTargetVmg' in d) {
      return Math.round(Math.abs(100 * d.deviceVmg / d.deviceTargetVmg));
    }
    return 0;
}

function vmgAtPoint(p) {
  if ('deviceVmg' in p) {
    return Math.abs(p.deviceVmg);
  }
  return undefined;
}

angular.module('www2App')
  .controller('MapCtrl', function ($scope, $stateParams, userDB, boatList,
                                   ModalService, $timeout, $http, $interval,
                                   $state, $location, $window, Auth) {

    var defaultColor = '#FF0033';
    var defaultTaillength = 300;

    $scope.tailLength = defaultTaillength;
    $scope.toggleVMG = ($location.search().queue && !$location.search().tailColor) || $location.search().allTrack ? true : false;
    $scope.toggleTail = $location.search().queue ? true : false;
    $scope.sections = {
      showPerfSpeed: false,
      showWind: false,
      showDetails: false
    };
    $scope.containers = {
      showInfoGroup: false,
      showSidebar: true,
      showGraph: true
    };
    $scope.boat = { _id: $stateParams.boatId, name: 'loading' };

    $scope.slider = {
      options: {
        floor: 30,
        ceil: 10800,
        step: 1,
        minLimit: 30,
        maxLimit: 10800,
      }
    };

    $scope.tabs = [
      {
        name: 'res-graph',
        icon: 'fa-area-chart'
      },
      {
        name: 'res-perf',
        icon: 'fa-dashboard'
      },
      {
        name: 'res-wind',
        icon: 'fa-flag'
      },
      {
        name: 'res-details',
        icon: 'fa-list'
      },
      {
        name: 'res-photos',
        icon: 'fa-photo'
      }
    ];

    var setLocationTimeout;
    function setLocation() {
      setSelectTime();
      function delayed() {
        setLocationTimeout = undefined;
        var search = '';
        if ($scope.mapLocation) {
          var l = $scope.mapLocation;
          search += 'l=' + l.x +',' + l.y + ',' + l.scale;
        }
        if ($scope.selectedCurve) {
          search += '&c=' + $scope.selectedCurve;
        }
        if ($location.search().queue) {
          search += '&queue=' + $location.search().queue;
        }
        if ($location.search().showLinks) {
          search += '&showLinks=' + $location.search().showLinks;
        }
        if ($location.search().tailColor) {
          search += '&tailColor=' + $location.search().tailColor;
        }
        if ($location.search().allTrack) {
          search += '&allTrack=' + $location.search().allTrack;
        }
        if (typeof $scope.currentTime !== 'undefined' && !isNaN($scope.currentTime)) {
          search += '&t=' + $scope.currentTime.getTime();
        }
        if (typeof $scope.currentTime === 'undefined' && $location.search().t != null) {
          search += '&t=' + $location.search().t;
          $scope.currentTime = new Date(parseInt($location.search().t));
        }
        
        $location.search(search).replace();
      }

      if (setLocationTimeout) {
        $timeout.cancel(setLocationTimeout);
      }
      setLocationTimeout = $timeout(delayed, 1000);
    }

    function setSelectTime() {
      if ($scope.selectedCurve) {
        $scope.startTime = curveStartTime($scope.selectedCurve);
        $scope.endTime = curveEndTime($scope.selectedCurve);
        if (!$scope.timeSelection) {
          $scope.timeSelection = {
            start: $scope.startTime,
            end: $scope.endTime
          };
        }
      }
    }

    function parseParams() {
      if ($location.search().c) {
        $scope.selectedCurve = $location.search().c;
        setSelectTime();
      }

      if ($location.search().l) {
        var entries = $location.search().l.split(',');
        $scope.mapLocation = {
          x: parseFloat(entries[0]),
          y: parseFloat(entries[1]),
          scale: parseFloat(entries[2])
        };
      }

      if($location.search().queue) {
        $scope.tailLength = $location.search().queue;
      }
    }

    parseParams();
    // Catches browser history navigation events (back,..)
    $scope.$on('$locationChangeSuccess', parseParams);

    boatList.boat($stateParams.boatId).then(function (boat) {
      $scope.boat = boat;
    });
    
    $scope.showModal = function() {
      ModalService.isVisible = true;
      ModalService.showModal({
        templateUrl: "app/share/share.map.social.html",
        controller: "ShareCtrl"
      });
    };

    $scope.eventList = [];
    $scope.users = {};
    $http.get('/api/events', { params: {
        b: $scope.boat._id,
        A: ($scope.startTime ? $scope.startTime.toISOString() : undefined),
        B: ($scope.endTime ? $scope.endTime.toISOString() : undefined)
      }})
      .success(function(data, status, headers, config) {
        if (status == 200) {
          var times= {};
          for (var i in data) {
            var event = data[i];
            
            // Parse date
            event.when = new Date(event.when);

            // Fetch user details
            userDB.resolveUser(event.author, function(user) {
              $scope.users[user._id] = user;
            });

            // Remove duplicates
            var key = "" + event.when.getTime();
            if (!(key in times)) {
              times[key] = 1;
              $scope.eventList.push(event);
            }
          }
        }
      });

    $scope.plotField = 'devicePerf';

    $scope.plotFieldLabels = {
      'gpsSpeed' : 'Speed over ground (GPS)',
      'devicePerf' : 'VMG performance',
      'aws' : 'Apparent wind speed',
      'tws' : 'True wind speed (Anemomind)',
      'externalTws' : 'True wind speed (onboard instruments)',
      'watSpeed': 'Water speed',
      'deviceVmg': 'VMG',
      'deviceTargetVmg': 'Target VMG'
      // those can't be displayed because they are angles:
      // awa deviceTwdir externalTwa gpsBearing magHdg
    };

    $scope.isPlaying = false;
    var animationTimer;

    $scope.togglePlayPause = function() {
      $scope.isPlaying = !$scope.isPlaying;
    }

    var lastPositionUpdate = new Date();

    $scope.$watch('isPlaying', function(newVal, oldVal) {
      if (newVal != oldVal) {
        if (newVal) {
          lastPositionUpdate = new Date();
          animationTimer = $interval(updatePosition, 100);
        } else {
          $interval.cancel(animationTimer);
        }
      }
    });

    $scope.$watch('toggleVMG', function(newVal, oldVal) {
      if (newVal != oldVal) {        
        if(newVal) {
          $location.search('tailColor',null);
          $location.search('allTrack',1);
        }          
        else {
          $location.search('tailColor',defaultColor);
          $location.search('allTrack',0);
        }          

        refreshMap();
      }
    });
    
    $scope.$watch('toggleTail', function(newVal, oldVal) {
      if (newVal != oldVal) {
        var queueVal = !newVal ? null : ($scope.tailLength ? $scope.tailLength : defaultTaillength);
        $location.search('queue', queueVal);
                
        if(queueVal && !$scope.toggleVMG)
          $location.search('tailColor',defaultColor);

        refreshMap();
      }
    });

    $scope.$watch('tailLength', function(newVal, oldVal) {
      if (newVal != oldVal) {
        var queueVal = !newVal ? null : newVal;
        $scope.toggleTail = !newVal ? false : true;
        
        $location.search('queue', queueVal);

        if(queueVal && !$scope.toggleVMG)
          $location.search('tailColor',defaultColor);

        refreshMap();
        changeVisibility();
      }
    });


    function endTime() {
      if ($scope.selectedCurve) {
        return curveEndTime($scope.selectedCurve);
      } else {
        return $scope.plotData[$scope.plotData.length - 1]['time'];
      }
    }

    function startTime() {
      if ($scope.selectedCurve) {
        return curveStartTime($scope.selectedCurve);
      } else {
        return $scope.plotData[0]['time'];
      }
    }

    function updatePosition() {
      var now = new Date();
      if(!$scope.currentTime){
        $scope.currentTime = new Date($scope.plotData[0]['time']);
      } else {
        var delta = (now.getTime() - lastPositionUpdate.getTime());
        delta *= $scope.replaySpeed;
        $scope.currentTime = new Date($scope.currentTime.getTime() + delta);

        if ($scope.currentTime >= endTime()) {
          $scope.currentTime = startTime();
        }
      }
      lastPositionUpdate = now;
    }

    // Refreshes the map by triggering the $watch of currentTime
    var refreshMap = function() {
      if($scope.currentTime)
        $scope.currentTime = new Date($scope.currentTime.getTime());
    };

    $scope.$watch('mapLocation', setLocation);
    $scope.$watch('selectedCurve', setLocation);

    var pointAtTime = function(time) {
      if (!time || !$scope.plotData || $scope.plotData.length < 2) {
        return {};
      }

      // TODO: move this function in a library.
      var binarySearch = function(list, item) {
        var min = 0;
        var max = list.length - 1;
        var guess;

        while ((max - min) > 1) {
            guess = Math.floor((min + max) / 2);

            if (list[guess].time < item) {
                min = guess;
            }
            else {
                max = guess;
            }
        }

        return [min, max];
      };

      var bounds = binarySearch($scope.plotData, time);
      var delta = [
        Math.abs($scope.plotData[bounds[0]].time - time),
        Math.abs($scope.plotData[bounds[1]].time - time)];
      var s = (delta[0] < delta[1] ? 0 : 1);
      return $scope.plotData[bounds[s]];
    };

    function getPointValue(keys) {
      if ($scope.currentPoint == undefined) {
        return undefined;
      }
      for (var i in keys) {
        var key = keys[i];
        if (key in $scope.currentPoint) {
          return $scope.currentPoint[key];
        }
      }
      return undefined;
    }

    function twdir() {
      var result = getPointValue(['twdir']);
      if (result != undefined) {
        return result;
      }
      var twa = getPointValue(['twa', 'externalTwa']);
      if (twa == undefined) {
        return twa;
      }
      var bearing = getPointValue(['gpsBearing']);
      return bearing + twa;
    }

    $scope.currentPoint = pointAtTime($scope.currentTime);
    $scope.$watch('currentTime', function(time) {
      $scope.currentPoint = pointAtTime(time);

      $scope.vmgPerf = perfAtPoint($scope.currentPoint);
      $scope.awa = getPointValue(['awa']);
      $scope.aws =  getPointValue(['aws']);
      $scope.twa = getPointValue(['twa', 'externalTwa']);
      $scope.tws =  getPointValue(['tws', 'externalTws']);
      $scope.gpsSpeed = getPointValue(['gpsSpeed']);
      $scope.twdir = twdir();
      $scope.gpsBearing = getPointValue(['gpsBearing']);
      $scope.deviceVmg = getPointValue(['vmg', 'deviceVmg']);
      if ($scope.deviceVmg) {
        $scope.deviceVmg = Math.abs($scope.deviceVmg);
      }
      $scope.deviceTargetVmg = getPointValue(['deviceTargetVmg']);

      setLocation();
    });

    $scope.replaySpeed = 8;
    $scope.slower = function() { $scope.replaySpeed /= 2; }
    $scope.faster = function() {
      var speed = $scope.replaySpeed * 2;  
      if(speed > 512)
        return false;

      $scope.replaySpeed = speed;
    }
    $scope.cutBefore = function() {
      if ($scope.selectedCurve && $scope.currentTime) {
        $scope.selectedCurve = makeCurveId(
            $scope.boat._id,
            $scope.currentTime,
            $scope.endTime);
        setSelectTime();
        $location.search('c', $scope.selectedCurve);
      }
    };
    $scope.cutAfter = function() {
      if ($scope.selectedCurve && $scope.currentTime) {
        $scope.selectedCurve = makeCurveId(
            $scope.boat._id,
            $scope.startTime,
            $scope.currentTime);
        setSelectTime();
        $location.search('c', $scope.selectedCurve);
      }
    };


    $scope.mapActive = true;
    $scope.graphActive = true;
    $scope.sideBarActive = true;

    // The following code handles responsiveness for small screen,
    // by hiding/showing the map, graph, and sidebar according to
    // window size.
    var verticalSizeThreshold = 600;
    var horizontalThreshold = 800;

    var mapScreenContainer = angular.element('.mapScreenContainer');
    var width = function() {
      return mapScreenContainer.width();
    };
    var height = function() {
      return mapScreenContainer.height();
    };


    // Angular Tabs only toggles by switching from 1 tab to another
    // This will allow to toggle the current tab by hiding it or not.
    // Why? For the map to have more viewable space.
    $scope.currentTab = null;
    $scope.showTabContent = true;
    $scope.selectTab = function(selectedIndex) {
      if ($scope.currentTab !== selectedIndex) {
        $scope.currentTab = selectedIndex;
        $scope.showTabContent = true;
      } else {
        $scope.showTabContent = !$scope.showTabContent;
      }
      delayedApply();
    }


    // Toggling the visibility of components change their size.
    // However, in HTML, there is no way to bind to a div resize event.
    // To avoid having to poll for size changes in a timer, when we
    // do an action that may cause resizes, we also tell angular to
    // watch for resizes after the effect of the resize change have been
    // applied.
    var delayedApply = function() {
      setTimeout(function() { $scope.$apply(); }, 10);
    };
    $scope.refreshGraph = function() {
      delayedApply();
    }
    $scope.activateMap = function() {
      $scope.mapActive = true;
      $scope.graphActive = (height() >= verticalSizeThreshold);
      $scope.sideBarActive = (width() >= horizontalThreshold);
      delayedApply();
    };

    $scope.activateGraph = function() {
      $scope.graphActive = true;
      $scope.mapActive = (height() >= verticalSizeThreshold);
      $scope.sideBarActive = (width() >= horizontalThreshold);
      delayedApply();
    };

    $scope.activateSideBar = function() {
      $scope.sideBarActive = true;
      if (width() < horizontalThreshold) {
        $scope.mapActive = false;
        $scope.graphActive = false;
      }
      delayedApply();
    };

    $scope.$watch(function(){
       return { width: width(), height: height() };
    }, function(value) {
       if (value.width < horizontalThreshold) {
         if (!$scope.containers.showGraph) {
           // If in desktop, the Graph is hidden
           // then when switching to mobile,
           // the Graph should be visible at start
           $scope.containers.showGraph = true;
         }
         if (!$scope.showTabContent) {
           // If in mobile, the Tab contents are hidden
           // then switch to desktop,
           // then switch again to mobile, it should be visible
           $scope.showTabContent = true;
         }
       } else {
         // If the screen got large enough, show the sidebar,
         // and make sure either the map or the graph is active.
         // The vertical checks below might activate both.
         $scope.sideBarActive = true;
         if (!$scope.mapActive && !$scope.graphActive) {
           $scope.mapActive = true;
         }
       }

      if (value.height < verticalSizeThreshold) {
        if ($scope.mapActive && $scope.graphActive) {
          //$scope.graphActive = false;
        }
      } else {
        if ($scope.mapActive || $scope.graphActive) {
          $scope.mapActive = $scope.graphActive = true;
        }
      }
    }, true);

    // So by default, the bubble is hidden
    // If slider is controlled, the bubble is visible
    // After 3 seconds, the bubble is hidden again
    var visibilityTimeout;
    var changeVisibility = function() {
      $scope.bubbleState = true;
      function hideBubble() {
        $scope.bubbleState = false;
        visibilityTimeout = undefined;
      }
      if (visibilityTimeout) {
        $timeout.cancel(visibilityTimeout);
      }
      visibilityTimeout = $timeout(hideBubble, 3000);
    };

    $scope.navigate = function(where) {
      $window.history[where]();
    };

    $scope.canDownloadCsv = function() {
      return ($scope.boat && $scope.boat._id
          && $scope.startTime && $scope.endTime);
    };

    $scope.downloadAsCsvLink = function() {
      if (!$scope.canDownloadCsv()) {
        return '';
      }

      var url = [
        '/api/export',
        $scope.boat._id,
        encodeTime($scope.startTime) + encodeTime($scope.endTime) + '.esa.log'
      ].join('/');
      if (Auth.isLoggedIn() && !$scope.boat.publicAccess) {
        url += '?access_token=' + Auth.getToken();
      }
      return url;
    };
});

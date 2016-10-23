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
  .controller('MapCtrl', function ($scope, $stateParams, userDB, $timeout,
                                   $http, $interval, $state, $location) {

    $scope.toggleVMG = false;
    $scope.toggleTail = $location.search().queue ? true : false;
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
        if ($location.search().preview) {
          search += '&preview=' + $location.search().preview;
        }
        if ($location.search().queue) {
          search += '&queue=' + $location.search().queue;
        }
        if ($location.search().tailColor) {
          search += '&tailColor=' + $location.search().tailColor;
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
      $scope.startTime = curveStartTime($scope.selectedCurve);
      $scope.endTime = curveEndTime($scope.selectedCurve);
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

    $http.get('/api/boats/' + $stateParams.boatId)
    .success(function(data, status, headers, config) {
      $scope.boat = data;

      var aveSpeedText = '32 Kts';
      var windBlowedText = '22 Kts';
      var performanceText = '91%';
      $scope.shareText = '"'+$scope.boat.name+'" and her team made a great performance with an average speed of '+aveSpeedText+'. The wind blowed at '+windBlowedText+'. Anemomind calculated a global performance of '+performanceText+'.';
      $scope.shareText += '\n\nAdd text ...'
    });

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
    
    $scope.$watch('toggleTail', function(newVal, oldVal) {
      if (newVal != oldVal) {
        var queueVal = !newVal ? null : ($scope.tailLength ? $scope.tailLength : 300);
        $location.search('queue', queueVal);
        updatePosition();
      }
    });

    $scope.$watch('tailLength', function(newVal, oldVal) {
      if (newVal != oldVal) {
        var queueVal = !newVal ? null : newVal;
        $scope.toggleTail = !newVal ? false : true;
        
        $location.search('queue', queueVal);
        updatePosition();
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
    $scope.faster = function() { $scope.replaySpeed *= 2; }
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
         if ($scope.mapActive && $scope.sideBarActive) {
           // If the screen becomes to small for both
           // the side bar and the map/graph container,
           // we hide the side bar.
           //$scope.sideBarActive = false;
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
});
'use strict';

angular.module('www2App')
  .directive('perfplot', function ($timeout, $q, $http) {
    return {
      templateUrl: 'app/perfplot/perfplot.html',
      template: '',
      restrict: 'E',
      scope: {
        currentTime: '=',
        boat: '=boat',
        startTime: '=',
        endTime: '='
        //channel: '=channel',
        //source: '=source'
      },
      link: function ($scope, element, attrs) {
        $scope.channelList = [];
        $scope.sourceList = [];
        $scope.channel = 'gpsSpeed';
        $scope.source = undefined;

        var graph = new Graph(element[0].querySelector('.graph-container'),
                              function(zoom, tileno) { });

        var samplesPerTile = graph.loader.samplesPerTile;

        var chartApiUrl = "/api/chart";

        var fieldInTile = function(field, index, tile) {
          if (!tile || !tile.count || !tile[field] || tile[field][index] <= 0) {
            return undefined;
          }
          return tile[field][index];
        };

        // The following functions does 2 things:
        //   1. convert the tile format from what the server sent into what
        //      perfplot.js expects.
        //   2. make sure a single defined point surrounded by undefined values
        //      will get displayed.
        //
        //  At the border between undefined and defined values, because
        //  perfplot draws lines, not points, a single point would not be visible.
        //  So we extend it with the same value to the undefined bin next to it.
        var convertAndExtendData = function(zoom, tileno, data) {
          var result= [];
          for (var t in data) {
            var tile = data[t];
            var tileTimeSec = tileno << zoom;
            var tileDur = 1 << zoom;

            var lastHasBeenExtended = false;
            for (var s = 0 ; s < samplesPerTile; ++s) {
              var count = tile.count ? tile.count[s] : undefined;
              var value = undefined;
              var min = undefined;
              var max = undefined;
              if (count > 0) {
                value = tile.mean ? tile.mean[s] : undefined;
                if (tile.min) {
                  min = tile.min[s];
                }
                if (tile.max) {
                  max = tile.max[s];
                }
              }

              if (result.length > 0) {
                var last = result[result.length - 1];
                if (!lastHasBeenExtended
                    && value === undefined && last.value !== undefined) {
                  value = last.value;
                  min = last.low;
                  max = last.high;
                  lastHasBeenExtended = true;
                } else {
                  lastHasBeenExtended = false;
                }
              } 

              var obj = {
                value: value,
                time: new Date((tileTimeSec + (s/samplesPerTile) * tileDur) * 1000)
              };
              if (tile.min) {
                obj.low = min;
              }
              if (tile.max) {
                obj.high = max;
              }

              result.push(obj);
            }
          }
          return result;
        };

        var makeUrl = function(channel, source, zoom, tileno) {
          return [ chartApiUrl, $scope.boat, zoom, tileno,
            encodeURIComponent(channel),
            encodeURIComponent(source)].join('/');
        };

        var fetchTile = function(zoom, tileno) {
          var url = makeUrl($scope.channel, $scope.source, zoom, tileno);

          $http.get(url).then(function(data) {
            graph.tileFetchSucceeded(zoom, tileno,
              convertAndExtendData(zoom, tileno, data.data));
          });
        };

        // The performance data is not directly computed on the server.
        // But it can be computed from vmg and targetVmg.
        // So we fetch both tiles, do the computation, are create a new tile
        // containing the computed performance.
        var fetchTilesForPerf = function(zoom, tileno) {
          $q.all([
            $http.get(makeUrl('vmg', $scope.source, zoom, tileno)),
            $http.get(makeUrl('targetVmg', $scope.source, zoom, tileno))])
          .then(function(data) {
            var vmgData = data[0].data[0];
            var targetVmgData = data[1].data[0];

            var tileTimeSec = tileno << zoom;
            var tileDur = 1 << zoom;

            var perfTile = {
              count: new Array(samplesPerTile),
              mean: new Array(samplesPerTile),
              min: new Array(samplesPerTile),
              max: new Array(samplesPerTile)
            };
            for (var i = 0; i < samplesPerTile; ++i) {
              var count = 
                Math.min(fieldInTile('count', i, vmgData) || 0,
                         fieldInTile('count', i, targetVmgData) || 0);
              if (count > 0 && Math.abs(targetVmgData.mean[i]) > .1) {
                perfTile.mean[i] =
                  Math.abs(100 * vmgData.mean[i] / targetVmgData.mean[i]);
                perfTile.min[i] =
                  Math.abs(100 * vmgData.min[i] / targetVmgData.max[i]);
                perfTile.max[i] =
                  Math.abs(100 * vmgData.max[i] / targetVmgData.min[i]);
                perfTile.count[i] = count;

              } else {
                perfTile.mean[i] = 0;
                perfTile.count[i] = 0;
              }
            }
            graph.tileFetchSucceeded(zoom, tileno,
              convertAndExtendData(zoom, tileno, [perfTile]));
          });
        };
        
        var currentSource, currentChannel;

        var updateTimeBounds = function() {
          if (!currentSource || !currentChannel) {
            return;
          }

          var source = $scope.channels[currentChannel][currentSource];
          var d =graph.x.domain();
          var newDomain =
            [new Date(source.first), new Date(source.last)];
          if ($scope.startTime instanceof Date
              && $scope.startTime.getTime() > 10000) {
            newDomain[0] = new Date($scope.startTime);
          }
          if ($scope.endTime instanceof Date
              && $scope.endTime.getTime() > 10000) {
            newDomain[1] = new Date($scope.endTime);
          }

          graph.setTimeBounds(newDomain[0], newDomain[1]);
          graph.draw();
        };

        graph.onZoom = function() {
          var d =graph.x.domain();
          $scope.startTime = new Date(d[0]);
          $scope.endTime = new Date(d[1]);
        };

        var selectionChanged = function() {
          if (currentSource !== $scope.source
              || currentChannel !== $scope.channel) {
            currentSource = $scope.source;
            currentChannel = $scope.channel;

            if (!currentChannel || !currentSource) {
              return;
            }
            graph.loader.setFetchTile(
              currentChannel != 'vmgPerf' ? fetchTile : fetchTilesForPerf);
            graph.preparedRange = undefined;

            var source = $scope.channels[currentChannel][currentSource];
            if (!source) {
              return;
            }

            updateTimeBounds();
          }
        };

        graph.onTimeClick = function(time) {
          $timeout(function() {
            $scope.currentTime = time;
          });
        };

        var fetchChannelList = function() {
          $http.get(chartApiUrl + '/' + $scope.boat).then(function(data) {
            $scope.channels = (data && data.data ? data.data.channels : { });
            $scope.channelList = [];
            if ('vmg' in $scope.channels && 'targetVmg' in $scope.channels) {
              var source = 'Simulated Anemomind estimator';
              var targetSource = $scope.channels.targetVmg[source];
              $scope.channels.vmgPerf = {};
              $scope.channels.vmgPerf[source] = {
                first: targetSource.first,
                last: targetSource.last
              };
              // make sure it is the first in the list.
              $scope.channelList.push('vmgPerf');
            }

            for (var c in $scope.channels) {
              if (c != 'vmgPerf') {
                $scope.channelList.push(c);
              }
            }
            $scope.channel = $scope.channelList.length > 0 ?
              $scope.channelList[0] : undefined;
          });
        };
        fetchChannelList();

        $scope.$watch('boat', function(boat, oldBoat) {
          if (boat !== oldBoat) {
            fetchChannelList();
          }
        });
        $scope.$watch('channel', function(chan, oldChan) {
          if (!$scope.channels) {
            return;
          }
          $scope.sourceList = [];
          var best;
        
          for (var s in $scope.channels[chan]) {
            var source = $scope.channels[chan][s];
            if (source) {
              $scope.sourceList.push(s);

              var prio = +source.priority;
              if (!best || (best.prio < prio)) {
                best = { prio: prio, source: s};
              }
            }
          }
          if (best) {
            $scope.source = best.source;
          } else {
            $scope.source = undefined;
          }
          selectionChanged();
        });
        $scope.$watch('source', selectionChanged);
        $scope.$watch('startTime', updateTimeBounds);
        $scope.$watch('endTime', updateTimeBounds);

        $scope.$watch('currentTime', function(newValue, oldValue) {
          if (newValue != undefined) {
            graph.setTimeMarks([newValue]);
          } else {
            graph.setTimeMarks([]);
          }
        }, true);

        // Watch for resize
        $scope.$watch(
          function () {
            return {
              w: element.width(),
              h: element.height()
            };
          },
          function (newValue, oldValue) {
            if (newValue.w != oldValue.w || newValue.h != oldValue.h) {
              graph.draw();
            }
          },
          true // deep object compare
        );
      }
    };
  });

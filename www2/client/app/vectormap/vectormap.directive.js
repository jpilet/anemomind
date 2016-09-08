'use strict';

angular.module('www2App')
  .directive('vectormap', function ($timeout, $window, $http, userDB, boatList, Auth) {
    return {
      template: '<canvas style="width:100%;height:100%"></canvas>',
      restrict: 'EA',
      link: function (scope, element, attrs) {
        var canvas;

        function initializeCanvas() {
          canvas = new CanvasTilesRenderer({
            canvas: element.children()[0],
            url: function(scale, x, y) { 
              // The token corresponds to account anemojp on mapbox.
	      /*
              return "http://a.tiles.wmflabs.org/bw-mapnik/"
                + scale + "/" + x + "/" + y + ".png";
              */
              return "//api.tiles.mapbox.com/v4/anemojp.d4524095/"
                + scale + "/" + x + "/" + y
                + ".png32?access_token="
                + "pk.eyJ1IjoiYW5lbW9qcCIsImEiOiJ3QjFnX00wIn0.M9AEKUTlyhDC-sMM9a0obQ";
            },
            maxNumCachedTiles: 256,
            initialLocation: scope.mapLocation,
            onLocationChange: function(canvasTilesRenderer) {
              $timeout(function() {
                scope.mapLocation = canvasTilesRenderer.getLocation();
              });
            }
          });

          scope.pathLayer = new VectorTileLayer({
            maxNumCachedTiles: 512,
            token: Auth.getToken()
          }, canvas);
          canvas.addLayer(scope.pathLayer);

          scope.$watch('eventList', function(newV, oldV) {
            if(newV.length > 0) {
              var eventList = [];
              for(var i in scope.eventList) {
                eventList.push({
                  "type": "Feature",
                  "properties": {
                    textPlacement: 'E',
                    hideIcon: false,
                    "icon": scope.eventList[i].photo ? "1" : "0"
                  },
                  "geometry": {
                    "type": "Point",
                    "osmCoordinates": {
                      x: scope.eventList[i].dataAtEventTime.pos[0],
                      y: scope.eventList[i].dataAtEventTime.pos[1]
                    }                    
                  }
                });
              }

              var geojson = 
              {
                "type": "FeatureCollection",
                "features": eventList
              };

              var poiLayer = new POILayer({
                renderer: canvas,
                geojson: geojson,
                onFeatureClic: function(feature, pos) {
                  feature.properties.text += " clicked";
                }
              });
              canvas.addLayer(poiLayer);
            }
          }, true);


          // A clic on the map selects a curve and sets current time.
          canvas.addClicHandler(function(pos) {
            var point = scope.pathLayer.findPointAt(
              pos.startWorldPos.x, pos.startWorldPos.y);
            if (point) {
              var dist = Utils.distance(
                  canvas.pinchZoom.viewerPosFromWorldPos(pos.startWorldPos),
                  canvas.pinchZoom.viewerPosFromWorldPos(point.point.pos[0],
                                                       point.point.pos[1]));
              // This is a threshold, in pixels, to select a point.
              if (dist < 20 * canvas.pixelRatio) {
                if (!scope.selectedCurve) {
                  scope.selectedCurve = point.curveId;
                }
                scope.currentTime = point.point.time;
                scope.currentPoint = point.point;
                canvas.refresh();

                scope.$apply();
                return true;
              }
            }
            return false;
          });

          if (scope.selectedCurve) {
              scope.pathLayer.selectCurve(scope.selectedCurve);
              scope.plotData = scope.pathLayer.getPointsForCurve(scope.selectedCurve);
          }

          scope.pathLayer.onSelect = function(curveId) {
            $timeout(function() {
              scope.selectedCurve = curveId;
              scope.plotData = scope.pathLayer.getPointsForCurve(curveId);
            });
          };

          scope.pathLayer.onDataArrived = function() {
            $timeout(function() {
              if (scope.selectedCurve) {
                scope.plotData = scope.pathLayer.getPointsForCurve(scope.selectedCurve);
              }
            });
          };

          function updateTileUrl() {
            function makeTileUrlFunc(boatId, starts, end) {
              if (starts) {
                return function (scale, x, y) {
                  return "/api/tiles/raw/"
                    + scale + '/' + x + '/' + y + '/' + boatId + '/'
                    + starts + '/' + end;
                };
              } else {
                return function (scale, x, y) {
                  return "/api/tiles/raw/"
                    + scale + '/' + x + '/' + y + '/' + boatId;
                };
              }
            }
            if (scope.selectedCurve) {
              var startsAfter = curveStartTimeStr(scope.selectedCurve);
              var endsBefore = curveEndTimeStr(scope.selectedCurve);
            } else {
              var startsAfter = undefined;
              var endsBefore = undefined;
            }
              
            scope.pathLayer.setUrl(makeTileUrlFunc(scope.boat._id,
                                                   startsAfter,
                                                   endsBefore));
          }

          scope.$watch('selectedCurve', function(newValue, oldValue) {
            if (newValue != oldValue) {
              updateTileUrl();
              scope.pathLayer.selectCurve(newValue);
              scope.plotData = scope.pathLayer.getPointsForCurve(newValue);
            }
          });

          scope.$watch('currentTime', function(newValue, oldValue) {
            if (newValue != oldValue) {
              scope.pathLayer.setCurrentTime(newValue);
            }
          });

          updateTileUrl();

          scope.$watch('boat._id', function(newValue, oldValue) {
            if (newValue && newValue != oldValue) {
              updateTileUrl();
            }
          });

          // Watch for resize
          scope.$watch(
            function () {
              return {
                w: element.width(),
                h: element.height()
              };
            },
            function (newValue, oldValue) {
              if (newValue.w != oldValue.w || newValue.h != oldValue.h) {
                canvas.resizeCanvas(newValue.w, newValue.h);
              }
            },
            true // deep object compare
          );
          angular.element($window).bind('resize', function () {
            scope.$apply();
          });
          canvas.resizeCanvas(element.width(), element.height());

          scope.$watch('mapLocation', function(newValue, oldValue) {
            function near(x,y) {
              if (!x || !y) return false;
              return Math.abs(y-x) < 1e-10;
            }
            if (!newValue) {
              return;
            }
            if (!oldValue
                || !near(newValue.x, oldValue.x)
                || !near(newValue.y, oldValue.y)
                || !near(newValue.scale, oldValue.scale)) {
              canvas.setLocation(newValue);
            }
          }, true);

        }  // function initializeCanvas

        scope.plotData = [];
        scope.currentTime = undefined;

        if (!scope.selectedCurve) {
          scope.selectedCurve = undefined;
          if (!scope.mapLocation) {
            scope.mapLocation = { x:.5, y:.5, scale: 1 };
          }
        } else {
          if (!scope.mapLocation) {
            scope.mapLocation = boatList.locationForCurve(scope.selectedCurve);
          }
        }

        if (scope.mapLocation) {
          initializeCanvas();
        } else {
          // We delay the canvas initialization until we have all required data.
          scope.$on('boatList:sessionsUpdated', function(event, data) {
            if (!canvas && !scope.mapLocation) {
              scope.mapLocation = boatList.locationForCurve(scope.selectedCurve);
              if (scope.mapLocation) {
                initializeCanvas();
              }
            }
          });
        }
      }
    };
  });

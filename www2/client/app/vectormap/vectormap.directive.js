'use strict';

angular.module('www2App')
  .directive('vectormap', function ($timeout, $window, $http,
                                    $httpParamSerializer, $location,
                                    userDB, boatList, Auth, Lightbox) {
    return {
      template: '<canvas style="width:100%;height:100%"></canvas>',
      restrict: 'EA',
      link: function (scope, element, attrs) {
        var canvas;

        function initializeCanvas() {
          canvas = new CanvasTilesRenderer({
            canvas: element.children()[0],
            token: Auth.getToken(),
            url: function(scale, x, y) { 
              var s = [ 'a', 'b', 'c' ][(scale + x + y) % 3];
              return "//stamen-tiles-" + s + ".a.ssl.fastly.net/toner-lite/"
                + scale + "/" + x + "/" + y + ".png";
	      /*
              return "http://a.tiles.wmflabs.org/bw-mapnik/"
                + scale + "/" + x + "/" + y + ".png";
              */
              /*
              // The token corresponds to account anemojp on mapbox.
              return "//api.tiles.mapbox.com/v4/anemojp.d4524095/"
                + scale + "/" + x + "/" + y
                + ".png32?access_token="
                + "pk.eyJ1IjoiYW5lbW9qcCIsImEiOiJ3QjFnX00wIn0.M9AEKUTlyhDC-sMM9a0obQ";
              */
            },
            maxNumCachedTiles: 256,
            initialLocation: scope.mapLocation,
            onLocationChange: function(canvasTilesRenderer) {
              $timeout(function() {
                scope.mapLocation = canvasTilesRenderer.getLocation();
              });
            }
          });

          scope.scaleLayer = new ScaleLayer({
            verticalPlacement: 'bottom',
            horizontalPlacement: 'right',
            margin: [5, 20],
            minCanvasWidth: 500
          }, canvas);
          canvas.addLayer(scope.scaleLayer);

          scope.copyrightLayer = new CopyrightLayer({
            text: 'Background by Stamen Design, CC BY 3.0. Data by OpenStreetMap (ODbL)',
            margin: [5, 1]
          }, canvas);
          canvas.addLayer(scope.copyrightLayer);
            
          scope.pathLayer = new VectorTileLayer({
            maxNumCachedTiles: 512,
            token: Auth.getToken()
          }, canvas);
          canvas.addLayer(scope.pathLayer);

          var images = [];
          var geojson = 
            {
              "type": "FeatureCollection",
              "features": []
            };

          var poiLayer = new POILayer({
            renderer: canvas,
            geojson: geojson,
            onFeatureClic: function(feature, pos) {
              selectEvent(feature);

              if(feature.properties.icon == "image") {
                Lightbox.openModal(images, feature.photoIndex);
              }
            }
          });
          var options = {
            width: 30,
            height: 30,
            ratioY: 1
          };
          poiLayer.loadIcon('comment', "/assets/images/chat.svg", options);
          poiLayer.loadIcon('image', "/assets/images/image.svg", options);

          function setTailTrack() {
            if ($location.search().queue) {
              var lengthTime = new Date(
                Math.abs(scope.currentTime.getTime() - 
                (parseInt($location.search().queue) * 1000)));

              var tailId = makeCurveId(
                scope.boat._id,
                lengthTime,
                scope.currentTime);

              var customContext = {
                lineColor: '#8b27ef',
                lineWidth: 3
              };

              
            }
          }
          

          scope.photoUrl = function(event, size) {
            var url = [
              '/api/events/photo/' + event.boat + '/' + event.photo,
              $httpParamSerializer({s : size, access_token: Auth.getToken()})
            ];
            return url.join('?');
          };

          var selectEvent = function(event) {
            if(event == null) {
              angular.element('#eventsContainer li').removeClass('selected');
              return true;
            }
            
            var sidebar = angular.element('.mapAndGraphAndSidebar #tabs');
            var target = angular.element('#eventsContainer li[data-id="'+event.id+'"]');
            var posTop = target.position();
            posTop = posTop.top;
            
            target.addClass('selected').siblings().removeClass('selected');
            sidebar.scrollTop(posTop);

            return true; 
          };

          scope.$watch('eventList', function(newV, oldV) {
            if(newV.length > 0) {
              geojson.features = [];
              for(var i in scope.eventList) {
                var event = scope.eventList[i];
                if (!event.dataAtEventTime || !event.dataAtEventTime.pos) {
                  continue;
                }
                var feature = {
                  "type": "Feature",
                  "id": scope.eventList[i]._id,
                  "properties": {
                    textPlacement: 'E',
                    hideIcon: false,
                    "icon": scope.eventList[i].photo ? "image" : "comment"
                  },
                  "geometry": {
                    "type": "Point",
                    "osmCoordinates": {
                      x: event.dataAtEventTime.pos[0],
                      y: event.dataAtEventTime.pos[1]
                    }                    
                  }
                };

                if (typeof scope.eventList[i].photo !== 'undefined'
                    && scope.eventList[i].photo
                    && scope.eventList[i].photo != null) {
                  var image = {
                    'url': scope.photoUrl(scope.eventList[i], ''),
                    'caption': scope.eventList[i].comment
                  };
                  feature.photoIndex = images.length;
                  images.push(image);
                }
                geojson.features.push(feature);
              }

              canvas.refreshIfNotMoving();
              
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
            if (scope.selectedCurve) {
              var startsAfter = curveStartTimeStr(scope.selectedCurve);
              var endsBefore = curveEndTimeStr(scope.selectedCurve);
            } else {
              var startsAfter = undefined;
              var endsBefore = undefined;
            }
              
            scope.pathLayer.buildUrl(scope.boat._id,startsAfter,endsBefore);
          }

          var selectEventByTime = function(time) {
            var bestTimeDiff = 300 * 1000;
            var bestEvent = null;

            if(scope.eventList.length > 0) {
              for(var i in scope.eventList) {
                //
                // avoid crash when data are inconsistent 
                if(!scope.eventList[i].dataAtEventTime){
                  continue;
                }
                var eventTime = new Date(scope.eventList[i].dataAtEventTime.time);
                var diffTime = Math.abs(eventTime.getTime() - time.getTime());

                if(diffTime < bestTimeDiff) {
                  bestTimeDiff = diffTime;
                  bestEvent = scope.eventList[i];
                }
              }

              if(bestEvent)
                selectEvent({'id': bestEvent._id});
              else
                selectEvent(null);
            }
          };

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
              scope.pathLayer.queueSeconds = $location.search().queue;
              scope.pathLayer.tailColor = $location.search().tailColor;
              scope.pathLayer.allTrack = $location.search().allTrack;
              selectEventByTime(newValue);
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

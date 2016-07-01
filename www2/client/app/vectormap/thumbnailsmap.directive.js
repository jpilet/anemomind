'use strict';

angular.module('www2App')
  .directive('thumbnailsmap', function ($timeout, $window, $parse, boatList, Auth) {
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
            token: Auth.getToken(),
            disabledZoom:true,
            maxNumCachedTiles: 256,
            initialLocation: scope.mapLocation,
            onLocationChange: function(canvasTilesRenderer) {
              $timeout(function() {
                scope.mapLocation = canvasTilesRenderer.getLocation();
              });
            }
          });


          scope.pathLayer = canvas.layers[1];

          if (scope.selectedCurve) {
              scope.pathLayer.selectCurve(scope.selectedCurve);
              scope.plotData = scope.pathLayer.getPointsForCurve(scope.selectedCurve);
          }


          scope.pathLayer.onDataArrived = function() {
            $timeout(function() {
              if (scope.selectedCurve) {
                scope.plotData = scope.pathLayer.getPointsForCurve(scope.selectedCurve);
              }
            });
          };


  
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
  
          element.css('height','100%');
          element.css('background-color','#ff0033 !important');
          angular.element($window).bind('resize', function () {
            scope.$apply();
          });

          canvas.resizeCanvas(element.width(), element.height());

        }  // function initializeCanvas


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


        scope.plotData = [];
        scope.currentTime = undefined;

        attrs.$observe('thumbnailsmap',function(curve) {
          var options={curve:curve,boatId:attrs.boatId};
          if(!options.curve||!options.boatId){
            // not ready yet
            return;
          }
          initializeCanvas();
          scope.selectedCurve=options.curve;
          scope.mapLocation = boatList.locationForCurve(options.curve);
          canvas.setLocation(scope.mapLocation);
          updateTileUrl();

        });


      }
    };
  });

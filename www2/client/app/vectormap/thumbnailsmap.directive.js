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
            token: Auth.getToken(),
            disabledZoom:true,
            maxNumCachedTiles: 256,
            initialLocation: scope.mapLocation
          });


          // init pathlayer
          scope.pathLayer = new VectorTileLayer({
            maxNumCachedTiles: 4,
            token: Auth.getToken()
          }, canvas);
          canvas.addLayer(scope.pathLayer);

  
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

          //
          // force height for this element   
          element.css('height','100%');
          angular.element($window).bind('resize', function () {
            scope.$apply();
          });

          canvas.resizeCanvas(element.width(), element.height());

        }  // function initializeCanvas


        function updateTileUrl() {

          var startsAfter = curveStartTimeStr(scope.selectedCurve);
          var endsBefore = curveEndTimeStr(scope.selectedCurve);            
          scope.pathLayer.selectCurve(scope.selectedCurve);
          scope.pathLayer.buildUrl(scope.boat._id,startsAfter,endsBefore);

        }



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

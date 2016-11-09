'use strict';
  

angular.module('www2App')
  .directive('thumbnailspng', function ($interpolate, $window, $parse, boatList, Auth) {
    return {
      restrict: 'A',
      link: function (scope, element, attrs) {

        //
        // use background image to keep proportion size  
        var style={
          'background-size':'cover', 
          'background-color':'transparent',
          'background-position': '50% 40%',
          'width':'100%',
          'height':'100%'          
        };    


 
        attrs.$observe('thumbnailspng',function(curve) {
          var options={curve:curve,boatId:attrs.boatId};
          if(!options.curve||!options.boatId){
            // not ready yet
            return;
          }
          scope.selectedCurve=options.curve;
          scope.boatId=options.boatId;
          scope.mapLocation = boatList.locationForCurve(options.curve);
          scope.start = curveStartTimeStr(scope.selectedCurve);
          scope.end = curveEndTimeStr(scope.selectedCurve);            
          scope.access_token=Auth.getToken();

          scope.width=300;
          scope.height=160;

          if (window.devicePixelRatio > 1) {
            scope.width *= 2;
            scope.height *= 2;
          }

          // create PNG
          // /api/map/$boatId/$x,$y,$scale/$start/$end/$w-$h.png
          var path=$interpolate("/api/map/{{boatId}}/{{mapLocation.x}},{{mapLocation.y}},{{mapLocation.scale}}/{{start}}/{{end}}/{{width}}-{{height}}.png?access_token={{access_token}}")(scope);

          //
          //
          style['background-image']='url('+path+')';
          element.css(style); 
        });


      }
    };
  });

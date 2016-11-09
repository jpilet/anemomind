'use strict';
angular.module('www2App')
  .directive('sessionPlot',sessionPlot);

//
// currently plot the strongestWindSpeed
// TODO make session plot more flexible to plot other session data
sessionPlot.$inject=['$compile','$timeout'];
function sessionPlot($compile,$timeout) {
  return {
    restrict: 'A',
    replace: false, 
    scope:{
      sessionPlot:'='
    },
    template: '<nvd3 options="options" data="plotarray"></nvd3>',
    priority:1,
    link: function(scope, element, attrs, ngModelCtrl) {
      var self=this;
      scope.options={
        chart: {
          type: 'lineChart',
          rightAlignYAxis:true,
          showLegend:false,
          useInteractiveGuideline:false,
          height:100,
          x: function(d){ return d.x; },
          y: function(d){ return d.y; },
          margin:{
            top:5,
            left:5,
            right:70,
            bottom:20
          },
          tooltip:{
            enabled:false
          },
          xAxis: {
            _axisLabel: 'Sessions'
          },
          yAxis: {
            _axisLabel: 'Distance (nmi)',
            tickFormat: function(d){
              return d3.format('.0f')(d)+' nm';
            }
          }
        }
      };

      scope.plotarray=[
        {
          values: [],
          key: 'Speed',
          color: '#ff0033'
        }
      ];

      scope.$watch('sessionPlot', function (sessionPlot) {
        if (scope['sessionPlot']) {
          // all available fields
          //  - strongestWindSpeed
          //  - avgWindSpeed
          //  - startTime
          //  - avgWindDir
          //  - trajectoryLength
          if(!sessionPlot||!sessionPlot.length){
            return;
          }
          // scope.options.chart.width=(element.width()*.9);
          scope.plotarray[0].values=[];
          var xAxis=0;
          sessionPlot.forEach(function (session) {
            scope.plotarray[0].values.push({series:0,x:xAxis++,y:session.strongestWindSpeed});
          })

        }
      });
    }
  };
}


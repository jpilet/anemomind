'use strict';

angular.module('www2App')
  .controller('VmgplotCtrl', function ($scope, $stateParams, $http, boatList) {
    $scope.message = 'Loading...';
    $scope.boat = boatList.boat($stateParams.boatId);

    $scope.$on('boatList:updated', function(event, boats) {
      $scope.boat = boatList.boat($stateParams.boatId);
    });

    $scope.options = {
      chart: {
        type: 'lineChart',
        height: 450,
        margin : {
          top: 20,
          right: 20,
          bottom: 40,
          left: 55
        },
        x: function(d){ return d.x; },
        y: function(d){ return d.y; },
        useInteractiveGuideline: true,
        dispatch: {
          stateChange: function(e){ console.log("stateChange"); },
          changeState: function(e){ console.log("changeState"); },
          tooltipShow: function(e){ console.log("tooltipShow"); },
          tooltipHide: function(e){ console.log("tooltipHide"); }
        },
        xAxis: {
          axisLabel: 'Wind Speed (knots)'
        },
        yAxis: {
          axisLabel: 'Boat VMG (knots)',
          tickFormat: function(d){
            return d3.format('.02f')(d);
          },
          axisLabelDistance: -10
        },
        interactiveLayer: {
          tooltip: {
            contentGenerator: function(d) {
              var html = '<p style="text-align:right">Wind: <b>'+d.value+" knots</b></p>";

              d.series.forEach(function(elem){
                html += '<p style="text-align:right"><span style="color:'+elem.color+'">'
                    +elem.key+" VMG</span> : <b>"+elem.value+" knots</b></p>";
              });
      
              return html;
            }
          }
        }
      },
      subtitle: {
        enable: true,
        text: 'Upwind and downwind VMG as a function of boat speed',
        css: {
          'text-align': 'center',
          'margin': '10px 13px 0px 7px'
        }
      },
    };

    $http.get('/api/boatstats/' + $stateParams.boatId)
    .success(function(data, status, headers, config) {
      $scope.message = '';
      var vmgtable = data.vmgtable;
      var upwindValues = [];
      var downwindValues = [];
      for (var i = 0; i < vmgtable.length; ++i) {
        var d = vmgtable[i];
        if (d.tws < 2) {
          // curves are usually too noisy below 2 knots
          continue;
        }
        if (d.up != undefined) {
          upwindValues.push({x: d.tws, y: d.up});
        }
        if (d.down != undefined) {
          downwindValues.push({x: d.tws, y: d.down});
        }
      } 
      $scope.vmgtable = [
        {
          values: upwindValues,
          key: 'Upwind',
          color: '#ff7f0e'
        },
        {
          values: downwindValues,
          key: 'Downwind',
          color: '#2ca02c'
        }
      ];
    })
    .error(function() {
      $scope.message = "can't load data";
    });

    
  });

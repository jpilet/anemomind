'use strict';

angular.module('www2App')
  .controller('PolarCtrl', function ($scope, $stateParams, $http,
                                              Auth, FileUploader, boatList) {
  var vmg = function(boatSpeed, twa) {
    if (boatSpeed == 0 || twa == 0) {
      return undefined;
    }
    return Math.abs(Math.cos(parseFloat(twa) * Math.PI / 180) * parseFloat(boatSpeed));
  };
  var absTwa = function(twa) {
    return (twa > 180 ? 360 - twa : twa);
  }

  var minMaxForSeries = function(func, series, axis) {
    axis = axis || 'y';
    return Math[func].apply(null, series.map(function(serie) {
      return Math[func].apply(null, serie.values.map(function(e) {
        return e[axis];
      }));
    }));
  };
  var maxForSeries =
      function(series, axis) { return minMaxForSeries('max', series, axis); }
  var minForSeries =
      function(series, axis) { return minMaxForSeries('min', series, axis); }

  var baseOptions = {
    chart: {
      type: 'anmMultiChart',
      height: 300,
      xDomain: [0, 20],
      margin : {
        top: 20,
        right: 20,
        bottom: 40,
        left: 55
      },
      useInteractiveGuideline: true,
      xAxis: {
        axisLabel: 'TWS [knots]',
        tickFormat: d3.format('.02f'),
      },
      xAxis1: {
        tickFormat: d3.format('.02f'),
      },
      yAxis1: {
        tickFormat: d3.format('.02f'),
        axisLabelDistance: -10
      },
      yAxis2: {
        tickFormat: d3.format('.02f'),
        axisLabelDistance: -10
      },
      scatters1: {
        xDomain: [0, 20]
      },
      callback: function(chart){ },
      interactiveLayer: {
        tooltip: {
          valueFormatter: d3.format('.02f')
        }
      }
    },
    title: {
      enable: true,
      text: ''
    },
  };


  $scope.boatId=$stateParams.boatId;
  $scope.name=$stateParams.name;

  boatList.boat($stateParams.boatId)
    .then(function (boat) { $scope.boat = boat; });

  $http.get('/api/perfstat/' + $stateParams.boatId
            + '/' + $stateParams.name)
    .success(function(data, status, headers, config) {
      $scope.perfstat = data;
      $scope.vmgPointsUp = [];
      $scope.vmgPointsDown = [];
      $scope.twaPointsUp = [];
      $scope.twaPointsDown = [];
      $scope.speedPointsUp = [];
      $scope.speedPointsDown = [];

      var labels = ['Upwind starboard', 'Downwind starboard', 'Downwind port', 'Upwind port'];
      var colors = ['#00d000', '#00d000', '#d00000', '#d00000'];

      for (var i = 0; i < 4; ++i) {
        var upDown = (i == 0 || i == 3 ? 'Up' : 'Down');
        $scope['vmgPoints' + upDown].push({
          values: data.esaVmgPoints
            .filter(function(d) { return d[i*2 + 1] != 0 && d[i*2+2] != 0; })
            .map(function (d) { return { x: d[0], y: vmg(d[i*2 + 1], d[i*2+2]) };
            }),
          key: labels[i],
          color: colors[i],
          type: 'scatter',
          yAxis: 1,
        });
        $scope['twaPoints' + upDown].push({
          values: data.esaVmgPoints
            .filter(function(d) { return d[i*2+2] != 0; })
            .map(function (d) { return { x: d[0], y: absTwa(d[i*2+2]) }; }),
          key: labels[i],
          color: colors[i],
          type: 'scatter',
          yAxis: 1,
        });
        $scope['speedPoints' + upDown].push({
          values: data.esaVmgPoints
            .filter(function(d) { return d[i*2+1] != 0; })
            .map(function (d) { return { x: d[0], y: d[i*2+1] }; }),
          key: labels[i],
          color: colors[i],
          type: 'scatter',
          yAxis: 1,
        });
      }
      var margin = 1.2;
      var maxVmg = Math.max(maxForSeries($scope.vmgPointsUp),
                            maxForSeries($scope.vmgPointsDown));
      if (maxVmg != undefined) {
        $scope.vmg_up_options.chart.yDomain1[1] = Math.ceil(maxVmg * margin)
      }

      var maxBoatSpeed = Math.max(maxForSeries($scope.speedPointsUp),
                                  maxForSeries($scope.speedPointsDown));
      $scope.speed_up_options.chart.yDomain1 =
      $scope.speed_down_options.chart.yDomain1 = [0, Math.ceil(maxBoatSpeed * margin)];

      var xMax;
      var xMin;
      ['vmg', 'speed', 'twa'].forEach(function(what) {
        ['Up', 'Down'].forEach(function(upDown) {
          var max = maxForSeries($scope[what + 'Points' + upDown], 'x');
          if (!xMax || xMax < max) { xMax = max; }
          var min = minForSeries($scope[what + 'Points' + upDown], 'x');
          if (!xMin || xMin > min) { xMin = min; }
        });
      });

      xMax = Math.ceil(xMax);
      xMin = Math.floor(xMin);

      ['vmg', 'speed', 'twa'].forEach(function(x) {
        ['_up', '_down'].forEach(function(direction) {
          $scope[x + direction + '_options'].chart.xDomain =
          $scope[x + direction + '_options'].chart.scatters1.xDomain = [xMin, xMax];
        });
      });
    });


    $scope.vmg_up_options = angular.copy(baseOptions);
    $scope.vmg_up_options.chart.yDomain1 = [ 0, 1 ];
    $scope.vmg_up_options.chart.yAxis1.axisLabel = 'VMG [knots]';
    $scope.vmg_up_options.title.text = 'VMG / TWS Up';
    $scope.vmg_down_options = angular.copy($scope.vmg_up_options);
    $scope.vmg_down_options.title.text = 'VMG / TWS Down';

    $scope.twa_down_options = angular.copy(baseOptions);
    $scope.twa_down_options.chart.yDomain1 = [ 90, 180 ];
    $scope.twa_down_options.chart.yAxis1.axisLabel = 'TWA [Degrees]';
    $scope.twa_down_options.title.text = 'TWA / TWS Down';
    $scope.twa_down_options.chart.xAxis.tickFormat =
    $scope.twa_down_options.chart.xAxis1.tickFormat = d3.format('.2f');
    $scope.twa_down_options.chart.yAxis1.tickFormat = d3.format('.0f');
    $scope.twa_down_options.chart.interactiveLayer.tooltip.valueFormatter =
          d3.format('.01f');

    $scope.twa_up_options = angular.copy($scope.twa_down_options);
    $scope.twa_up_options.chart.yDomain1 = [0, 90];
    $scope.twa_up_options.title.text = 'TWA / TWS Up';

    $scope.speed_up_options = angular.copy(baseOptions);
    $scope.speed_up_options.chart.yAxis1.axisLabel = 'Boat speed [knots]';
    $scope.speed_up_options.chart.yAxis1.tickFormat = d3.format('.0f');
    $scope.speed_up_options.title.text = 'Boat speed / TWS Up';

    $scope.speed_down_options = angular.copy($scope.speed_up_options);
    $scope.speed_down_options.title.text = 'Boat speed / TWS Down';

  $http.get('/api/perfstat/' + $stateParams.boatId)
    .success(function(data, status, headers, config) {
      $scope.perfstatIndex = data;
    });
});


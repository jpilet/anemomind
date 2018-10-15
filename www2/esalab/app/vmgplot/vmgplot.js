'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('vmgplot', {
        url: '/vmgplot/:boatId',
        templateUrl: 'app/vmgplot/vmgplot.html',
        controller: 'VmgplotCtrl'
      });
  });

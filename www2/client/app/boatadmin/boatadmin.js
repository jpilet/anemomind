'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('boatadmin', {
        url: '/boatadmin',
        templateUrl: 'app/boatadmin/boatadmin.html',
        controller: 'BoatadminCtrl'
      });
  });

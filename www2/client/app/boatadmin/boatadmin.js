'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('boatadmin', {
        url: '/boatadmin',
        templateUrl: 'app/boatadmin/boatadmin.html',
        controller: 'BoatadminCtrl'
      })
      .state('boatadmin-updates', {
        url: '/boatadmin/updates',
        templateUrl: 'app/boatadmin/updates.html',
        controller: 'UpdatesCtrl'
      });
  });

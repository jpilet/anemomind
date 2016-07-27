'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
    $stateProvider
      .state('boxexec', {
        url: '/boatadmin/boxexec/:boatId?',
        templateUrl: 'app/boatadmin/boxexec/boxexec.html',
        controller: 'BoxexecCtrl'
      });
  });

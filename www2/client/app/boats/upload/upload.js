'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
      $stateProvider.state('boat-upload', {
        url: '/boats/:boatId/upload',
        templateUrl: 'app/boats/upload/upload.html',
        controller: 'BoatUploadCtrl'
      });
  });

'use strict';

angular.module('www2App')
  .config(function ($stateProvider) {
      $stateProvider.state('boat-upload', {
        url: '/boats/:boatId/upload',
        templateUrl: 'app/boats/upload/upload.html',
        controller: 'BoatUploadCtrl'
      });
  })
  .config(function ($stateProvider) {
      $stateProvider.state('boat-file-detail', {
        url: '/boats/:boatId/upload/:filename',
        templateUrl: 'app/boats/upload/file-detail.html',
        controller: 'BoatFileDetailCtrl'
      });
  });

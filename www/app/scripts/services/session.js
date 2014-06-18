'use strict';

angular.module('anemomindApp')
  .factory('Session', function ($resource) {
    return $resource('/api/session/');
  });

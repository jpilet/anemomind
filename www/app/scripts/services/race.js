'use strict';

angular.module('anemomindApp')
  .factory('Race', function ($resource) {
    return $resource('/api/races/:id', {
      id: '@id'
    }, { //parameters default
      update: {
        method: 'PUT',
        params: {}
      },
      get: {
        method: 'GET',
        isArray: true
      }
    });
  });

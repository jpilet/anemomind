'use strict';

angular.module('www2App')
  .factory('BoxExec', function ($resource) {
    return $resource('/api/boxexecs/:id/:controller', {
      id: '@_id'
    });
  });


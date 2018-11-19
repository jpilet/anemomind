'use strict';

angular.module('www2App')
  .controller('BoxexecCtrl', function ($scope, $stateParams, BoxExec, $resource) {
    var query;
    var Boat = $resource('/api/boats/:id', { id: '@_id' });

    if ($stateParams.boatId) {
      query = { boatId: $stateParams.boatId };
      $scope.boat = Boat.get({id: $stateParams.boatId});
    }
    $scope.boxexecs = BoxExec.query(query);
  });

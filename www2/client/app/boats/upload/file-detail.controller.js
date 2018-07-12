'use strict';

angular.module('www2App')
  .controller('BoatFileDetailCtrl', function ($scope, $stateParams, $http,
                                          Auth, FileUploader, boatList) {
    $scope.boat = { };
    $scope.isBoatAdmin = true;
    $scope.boatId=$stateParams.boatId;
    $scope.filename=$stateParams.filename;
    $scope.file = {};
    $scope.warnings = [];

    boatList.boat($stateParams.boatId)
      .then(function (boat) {
        $scope.boat = boat;
        var userid = Auth.getCurrentUser()._id;
        $scope.isBoatAdmin = Auth.isAdmin() || (boat.admins && boat.admins.indexOf(userid) > -1);
      });

    $scope.fetchFile = function() {
      $scope.details = {};
      $http.get('/api/files/' + $stateParams.boatId
                + '/' + $stateParams.filename).then(function(response) {
        var f = $scope.file = response.data;
        f.start = new Date(f.start);
        f.end = new Date(f.start.getTime() + f.duration_sec * 1000);
        if (f.data && !f.data.match(/pos/)) {
          $scope.warnings.push('Missing GPS data');
        }
        if (f.data && !f.data.match(/awa/) || !f.data.match(/aws/)) {
          $scope.warnings.push('Missing wind data');
        }
      });
    };

    $scope.fileIsGood = function(f) {
      return !!f.data;
    };
    $scope.fileIsBad = function(f) {
      return !f.data;
    };
    $scope.delete = function(f) {
      $http.delete('/api/files/' + $stateParams.boatId + '/' + f.name)
      .then(function() {
        $scope.files.splice($scope.files.indexOf(f), 1);
      });
    };

    $scope.fetchFile();
  });

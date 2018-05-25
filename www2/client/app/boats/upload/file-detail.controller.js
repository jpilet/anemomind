'use strict';

angular.module('www2App')
  .controller('BoatFileDetailCtrl', function ($scope, $stateParams, $http,
                                          Auth, FileUploader, boatList) {
    $scope.boat = { };
    $scope.isAdmin = Auth.isAdmin;
    $scope.boatId=$stateParams.boatId;
    $scope.filename=$stateParams.file;
    $scope.file = {};
    $scope.warnings = [];

    boatList.boat($stateParams.boatId)
      .then(function (boat) {
        $scope.boat = boat;
      });

    $scope.fetchFile = function() {
      $scope.details = {};
      $http.get('/api/files/' + $stateParams.boatId
                + '/' + $stateParams.file).then(function(response) {
        const f = $scope.file = response.data;
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

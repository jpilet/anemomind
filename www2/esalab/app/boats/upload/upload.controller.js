'use strict';

angular.module('www2App')
  .controller('BoatUploadCtrl', function ($scope, $stateParams, $http,
                                          Auth, FileUploader, boatList) {
    $scope.uploader = new FileUploader({
      url: '/api/files/' + $stateParams.boatId,
      headers: { 
        "Authorization": "Bearer " + Auth.getToken()
      },
      autoUpload: true
    });
    $scope.boat = { };
    $scope.isBoatAdmin = true;
    $scope.boatId=$stateParams.boatId;
    $scope.files = [];

    boatList.boat($stateParams.boatId)
      .then(function (boat) {
        $scope.boat = boat;
        var userid = Auth.getCurrentUser()._id;
        $scope.isBoatAdmin = Auth.isAdmin() || (boat.admins && boat.admins.indexOf(userid) > -1);
      });

    $scope.fetchFileList = function() {
      $http.get('/api/files/' + $stateParams.boatId).then(function(response) {
        $scope.files = response.data;
        $scope.files.forEach(function(f) {
          if ('start' in f) {
            f.start = new Date(f.start);
            f.complete = f.data && f.data.match(/awa/) && f.data.match(/pos/);
          } else if (f.type && f.type == 'ESA Polar') {
            f.complete = true;
          }
        });
      });
    };

    $scope.uploader.onSuccessItem = function(fileItem, response, status, headers) {
      console.log('Uploaded: ', response.file);
      fileItem.remove();
      $scope.fetchFileList();
    };

    $scope.fileIsGood = function(f) {
      return (f.data != undefined || f.type == 'ESA Polar');
    };
    $scope.fileIsBad = function(f) {
      return !$scope.fileIsGood(f);
    };
    $scope.delete = function(f) {
      $http.delete('/api/files/' + $stateParams.boatId + '/' + f.name)
      .then(function() {
        $scope.files.splice($scope.files.indexOf(f), 1);
      });
    };

    $scope.fetchFileList();
  });

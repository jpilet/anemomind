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
    $scope.isAdmin = Auth.isAdmin;
    $scope.boatId=$stateParams.boatId;
    $scope.files = [];

    boatList.boat($stateParams.boatId)
      .then(function (boat) {
        $scope.boat = boat;
      });

    $scope.fetchFileList = function() {
      $http.get('/api/files/' + $stateParams.boatId).then(function(response) {
        $scope.files = response.data;
      });
    };

    $scope.uploader.onSuccessItem = function(fileItem, response, status, headers) {
      console.log('Uploaded: ', response.file);
      fileItem.remove();
      $scope.fetchFileList();
    };

    $scope.fetchFileList();
  });

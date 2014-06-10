'use strict';

angular.module('anemomindApp')
  .controller('UploadCtrl', function($scope, $upload) {
    $scope.onFileSelect = function($files) {
      //$files: an array of files selected, each file has name, size, and type.
      for (var i = 0; i < $files.length; i++) {
        var file = $files[i];
        $scope.upload = $upload.upload({
          url: 'api/upload', //upload.php script, node.js route, or servlet url
          // method: POST or PUT,
          // headers: {'header-key': 'header-value'},
          // withCredentials: true,
          data: {myObj: $scope.myModelObj},
          file: file, // or list of files: $files for html5 only
        }).progress(function(evt) {
          console.log('percent: ' + parseInt(100.0 * evt.loaded / evt.total));
        }).success(function(data, status, headers, config) {
          console.log('file stored successfully: ' + data);
        }).error(function(data) {
          console.log('failed to upload file: ' + data);
        });
        //.then(success, error, progress); 
        //.xhr(function(xhr){xhr.upload.addEventListener(...)})// access and attach any event listener to XMLHttpRequest.
      }
    };
  });
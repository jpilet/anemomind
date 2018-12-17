'use strict';

angular.module('www2App')
  .controller('UpdatesCtrl', function ($scope, Auth, $resource, $q) {
    $scope.isAdmin = Auth.isAdmin;

    $scope.fromVersion = 'v1.1';
    $scope.toVersion = 'v1.3';


    var Updates = $resource('/api/updates/');

    $scope.availableUpdates = Updates.query({});

    $scope.createUpdate = function() {
       var newUpdate = new Updates();
       newUpdate.to = $scope.toVersion;
       newUpdate.from = $scope.fromVersion;
       newUpdate.patchName = $scope.patchName;
       newUpdate.deploy = $scope.updateDeployCode;
       newUpdate.$save()
       .then(function() { $scope.availableUpdates = Updates.query({}); });
    };

    $scope.updateDeployCode = [
      "#!/bin/bash",
      "set -e",
      "date",
      "BUNDLE=''",
      "TARGET_VER=''",
      'cat "${BUNDLE}.split".* > "${BUNDLE}" || true',
      'rm "${BUNDLE}.split".* || true',
      'echo git bundle verify',
      "git -C /anemonode bundle verify \"${BUNDLE}\"",
      '#echo XX git reset',
      '#git -C /anemonode reset --hard 2>&1',
      '#echo XX git clean',
      '#git -C /anemonode clean -f -d 2>&1',
      'echo git pull',
      "git -C /anemonode pull \"${BUNDLE}\" \"${TARGET_VER}\" > /dev/null 2>&1",
      "sync",
      "echo done applying ${BUNDLE}",
      "git -C /anemonode rev-parse HEAD"
    ].join("\n");

    var makePatchName = function() {
      $scope.patchName = 'patch-' + $scope.fromVersion + '-' + $scope.toVersion + '.bundle';
    };
    makePatchName();
    $scope.$watch('fromVersion', makePatchName);
    $scope.$watch('toVersion', makePatchName);

    $scope.$watch('toVersion', function() {
       if ($scope.toVersion) {
         $scope.updateDeployCode = 
           $scope.updateDeployCode.replace(
               /TARGET_VER=.*/, "TARGET_VER='" + $scope.toVersion + "'");
       }
    });
    $scope.$watch('patchName', function() {
       if ($scope.patchName) {
         $scope.updateDeployCode = 
           $scope.updateDeployCode.replace(
               /BUNDLE=.*/,
               "BUNDLE='/home/anemobox/" + $scope.patchName + "'");
       }
    });
  });

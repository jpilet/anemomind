angular.module('starter.controllers', [])

.controller('DashCtrl', function($scope, $ionicPopup) {})

.controller('DevicesCtrl', function($scope, Devices) {
    var addedDevices = {};

    Devices.startScan(function(r) {
      //address, rssi, name, scanRecord
      var res = r.rssi + " " + r.name + " " + r.address;
      console.log('scan result: ' + res);
      if (r.address in addedDevices) {
        $scope.devices[addedDevices[r.address]] = r;
      } else {
        addedDevices[r.address] = $scope.devices.length;
        $scope.devices.push(r);
      }
      $scope.$apply();
    }, function(errorCode) {
      console.log('startScan error: ' + errorCode);
    });

  $scope.devices = [];
})

.controller('DeviceDetailCtrl', function($scope, $stateParams, $timeout, Devices) {
  $scope.device = {
    status: 'Connecting..',
    address: $stateParams.deviceAddress
  };

  $scope.disconnect = function() {
    if ($scope.device.handle) {
      Devices.disconnect($scope.device.handle);
    }
    $scope.device.status = "disconnected.";
    $scope.$apply();
  }
    
  Devices.connect($stateParams.deviceAddress,
                 function(info) { // success
                   $timeout(function() {
                     console.log('BLE connect status for device: '
                          + info.deviceHandle
                          + ' state: '
                          + info.state);
                     $scope.device.status = 'State: ' + info.state;
                     $scope.handle = info.deviceHandle;
                     $scope.$apply();
                   });
                 },
                 function(errCode) {  // failure
                   console.log('BLE connect error: ' + errorCode);
                   $scope.device.status = 'Error: ' + errCode;
                   $scope.$apply();
                 });
})

.controller('FriendsCtrl', function($scope, Friends) {
  $scope.friends = Friends.all();
})

.controller('FriendDetailCtrl', function($scope, $stateParams, Friends) {
  $scope.friend = Friends.get($stateParams.friendId);
})

.controller('AccountCtrl', function($scope) {
  $scope.settings = {
    enableFriends: true
  };
});

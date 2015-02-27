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
    status: 'Disconnected',
    address: $stateParams.deviceAddress,
    connected: false,
    services: []
  };

  $scope.disconnect = function() {
    if ($scope.device.handle) {
      Devices.disconnect($scope.device.handle);
    }
    $scope.device.status = "disconnected.";
    $scope.device.connected = false;
    $scope.$apply();
  }
    
  var discoverServices = function() {
    if ($scope.device.handle) {
      Devices.getServices(
        $scope.device.handle,
        function(services) {
          $scope.device.services = services;
          $scope.$apply();
        }
      );
    }
  };

  $scope.connect = function() {
    Devices.connect(
      $stateParams.deviceAddress,
      function(info) { // success
        console.log('BLE connect status for device: '
                           + info.deviceHandle
                           + ' state: '
                           + info.state);
        $scope.device.status = 'State: ' + info.state;
        if (info.state == 2) {
          $scope.device.connected = true;
        }
        $scope.device.handle = info.deviceHandle;
        $scope.$apply();
        if (info.state == 2) {
          discoverServices();
        }
      },
      function(errCode) {  // failure
        console.log('BLE connect error: ' + errorCode);
        $scope.device.status = 'Error: ' + errCode;
        $scope.$apply();
      }
    );
  };
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

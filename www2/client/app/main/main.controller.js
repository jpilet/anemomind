'use strict';

angular.module('www2App')
  .controller('MainCtrl', function ($scope, $http, socket) {
    $scope.awesomeThings = [];

    $http.get('/api/things').success(function(awesomeThings) {
      $scope.awesomeThings = awesomeThings;
      socket.syncUpdates('thing', $scope.awesomeThings);
    });

    $scope.addThing = function() {
      if($scope.newThing === '') {
        return;
      }
      $http.post('/api/things', { name: $scope.newThing });
      $scope.newThing = '';
    };

    $scope.deleteThing = function(thing) {
      $http.delete('/api/things/' + thing._id);
    };

    $scope.$on('$destroy', function () {
      socket.unsyncUpdates('thing');
    });
  })


  .controller("PathController", [ '$scope', function($scope) {
    angular.extend($scope, {
        center: {
            lat: 48,
            lng: 4,
            zoom: 4
        },
        paths: {
            p1: {
                color: '#008000',
                weight: 8,
                latlngs: [
                    { lat: 51.50, lng: -0.082 },
                    { lat: 48.83, lng: 2.37 },
                    { lat: 41.91, lng: 12.48 }
                ],
            }
        },
        markers: {
            london: {
                lat: 51.50,
                lng: -0.082,
                icon: {
                    iconUrl: 'examples/img/100x100_PNG/bigben100.png',
                    iconSize: [80, 80],
                    iconAnchor: [40, 80],
                    popupAnchor: [0, 0],
                    shadowSize: [0, 0],
                    shadowAnchor: [0, 0]
                }
            },
            paris: {
                lat: 48.83,
                lng: 2.37,
                icon: {
                    iconUrl: 'examples/img/100x100_PNG/eiffel100.png',
                    iconSize: [80, 80],
                    iconAnchor: [40, 60],
                    popupAnchor: [0, 0],
                    shadowSize: [0, 0],
                    shadowAnchor: [0, 0]
                }
            },
            roma: {
                lat: 41.91,
                lng: 12.48,
                icon: {
                    iconUrl: 'examples/img/100x100_PNG/colosseum100.png',
                    iconSize: [60, 60],
                    iconAnchor: [30, 40],
                    popupAnchor: [0, 0],
                    shadowSize: [0, 0],
                    shadowAnchor: [0, 0]
                }
            }
        },
        defaults: {
            scrollWheelZoom: false
        }
    });
}]);
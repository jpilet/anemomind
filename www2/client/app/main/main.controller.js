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

 .controller('PathController', [ '$scope', function($scope) {
  angular.extend($scope, {
    lausanne: {
      lat: 46.5198, 
      lng: 6.6335,
      zoom: 11
    },
    defaults: {
      tileLayer: "http://{s}.tiles.mapbox.com/v3/openplans.map-g4j0dszr/{z}/{x}/{y}.png",
      tileLayerOptions: {
        attribution: '© anemomind 2014',
        opacity: 1,
        detectRetina: true,
        reuseTiles: true,
      }
    },
    layers: {
      baselayers: {
        osm:{
          name: "Mapbox",
          type: "xyz",
          url: 'http://{s}.tiles.mapbox.com/v3/openplans.map-g4j0dszr/{z}/{x}/{y}.png',
          layerOptions: {
            subdomains: ['a', 'b', 'c'],
            attribution: '&copy; <a href="http://anemomind.com">anemomind</a> & <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors',
            continuousWorld: true
          }
        }
      },
      overlays: {
        sessions: {
          name:'Sesions',
          type: 'geoJSON',
          url:'../api/tiles/{z}/{x}/{y}/Irene',
          layerOptions: {
            style: {
              "color": "#00D",
              "fillColor": "#00D",
              "weight": 1.0,
              "opacity": 0.6,
              "fillOpacity": .2
            }
          },
          pluginOptions:{
            cliptiles: true
          }
        }
      }
    }
  });
}]);
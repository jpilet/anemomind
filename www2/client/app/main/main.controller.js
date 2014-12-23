'use strict';

angular.module('www2App')

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
          name:'Sessions',
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
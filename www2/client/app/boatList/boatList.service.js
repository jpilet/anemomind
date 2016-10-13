'use strict';

angular.module('www2App')
  .service('boatList', function (Auth, $http, $q,socket, $rootScope,$log) {
    var boats = [ ];
    var boatDict = { };
    var curves = { };
    var sessionsForBoats = {};

    //
    // promise for boats methods;
    var deferred = $q.defer();
    var promise=deferred.promise;


    //
    // update local dict and array of boats
    function updateBoatRepo(boat) {
      boatDict[boat._id] = boat;
      for (var i in boats) {
        if (boats[i]._id == boat._id) {
          boats[i] = boat;
          return;
         }
      }
      boats.push(boat);
    }    

    //
    // return the default boat to display at home
    function getDefaultBoat() {
      if(!boats.length){
        return {};
      }

      var bestBoat;
      var mostRecentSession;
      for (var boat in sessionsForBoats) {
        var sessions = sessionsForBoats[boat];
        for (var j in sessions) {
          var s = sessions[j];
          var time = new Date(s.endTime);
          if (!bestBoat || mostRecentSession < time) {
            mostRecentSession = time;
            bestBoat = boat;
          }
        }
      }
      return (bestBoat ? boatDict[bestBoat] : undefined);
    }

    function update() {
      //
      // if this promise is already deferred
      if(promise.$$state.status){
        deferred = $q.defer();  
        promise=deferred.promise;      
      }

      // Specifically do not ask for public boats.
      // We could have a user option to show public boats with:
      // $http.get('/api/boats?public=1')
      $http.get('/api/boats')
        .then(function(payload) {
          boats = payload.data;
          socket.syncUpdates('boat', boats);

          for (var i in payload.data) {
            var boat = payload.data[i];
            boatDict[boat._id] = boat;
          }
          $rootScope.$broadcast('boatList:updated', boats);

          //
          // chain session loading
          return $http.get('/api/session');
        })
        .then(function(payload) {
          sessionsForBoats = [];
          for (var i in payload.data) {
            if (payload.data[i].boat in sessionsForBoats) {
              sessionsForBoats[payload.data[i].boat].push(payload.data[i]);
            } else {
              sessionsForBoats[payload.data[i].boat] = [ payload.data[i] ];
            }
            curves[payload.data[i]._id] = payload.data[i];
          }
          //
          // time to resolve promise
          $rootScope.$broadcast('boatList:sessionsUpdated', sessionsForBoats);
          $log.log('-- boatList.update',boats.length);
          deferred.resolve(boats);            
        });

      return promise;
    }


    function save(id,boat) {
      var promise=$http.put('/api/boats/' + id, boat);
      promise.success(updateBoatRepo);
      return promise;
      
    }

    function addMember(id,invitation) {
      var promise=$http.put('/api/boats/' + id + '/invite', invitation);
      promise.success(updateBoatRepo);
      return promise;
    }


    function locationForCurve(curveId) {
      if (!(curveId in curves)) {
        return undefined;
      }
      var c = curves[curveId];
      return c.location;
    }

    //
    // init
    // update();
    $rootScope.$watch(Auth.isLoggedIn, function(newVal, oldVal) {
      if (newVal && newVal != oldVal) {
        //
        // force update boats
        $log.log('-- boatList.watch.isLoggedIn',newVal);
        update();    
      }
    });

    //
    // service result
    return {
      boat: function(id) { return boatDict[id]; },
      save: save,
      addMember:addMember,
      boats: function() { return promise; },
      sessions: function() { return $.extend({}, sessionsForBoats); },
      sessionsForBoat: function(boatId) { return sessionsForBoats[boatId]; },
      getCurveData: function(curveId) { return curves[curveId]; },
      getDefaultBoat: getDefaultBoat,
      locationForCurve: locationForCurve,
      update: update,
    };
  });

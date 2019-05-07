'use strict';

var app = angular.module('www2App')
  .controller('SessionCtrl', function ($scope, $stateParams, boatList, Auth, $log, $http, urlFriendlyFormFilter) {

    var scope = $scope;

    $scope.sessionId = $stateParams.sessionId;
    $scope.boatId = $stateParams.sessionId.substr(0, 24);

    // one of: unknown, ready, 'in-progress', 'not-computed'
    $scope.esaStatus = 'unknown';
    $scope.error = undefined;

    scope.isBoatAdmin = false;

    function updateSessions() {

      if(!scope.sessionId) return;

      boatList.boat(scope.boatId).then(function (boat) {
        scope.boat = boat;
        var userid=Auth.getCurrentUser()._id;
        scope.isBoatAdmin = Auth.isAdmin() || (boat.admins && boat.admins.indexOf(userid) > -1);
        scope.session = boatList.getCurveData(scope.sessionId);

        if (!scope.session) {
          return;
        }

        var session = scope.session;

        session.hasPhoto=scope.hasSocialActivity(session,'photos');
        session.hasComment=scope.hasSocialActivity(session,'comments');

        // keep session _id in photos as helper to link on viewmap
        if(session.hasPhoto){
          var start=new Date(session.startTime);
          var end=new Date(session.endTime);
          scope.boat.photos.forEach(function (photo) {
            var when=new Date(photo.when);
            if(start <= when && end >= when){
              photo.sid=session._id;
            }
          });
        }
      });
    }

    var loadPerfStat = function(response) {
      if (!response.data) {
        $scope.esaStatus = 'not-computed';
      } else {
        $scope.esaStatus = response.data.status || 'not-computed';
        $scope.error = response.data.error;
      }
    };

    $http.get('/api/perfstat/' + $scope.boatId + '/'
              + curveStartTimeStr($stateParams.sessionId)
              + '/' + curveEndTimeStr($stateParams.sessionId))
    .then(loadPerfStat,
    function (err) {
      console.log(err);
    })

    scope.$on('boatList:updated', updateSessions);
    scope.$on('boatList:sessionsUpdated', updateSessions);
    scope.$watch('boatId', updateSessions);
  
    //
    // get photo here
    // TODO make this a directive or an API available on top controller
    scope.photoUrl = function(bid,photo, size) {
      return '/api/events/photo/' + bid + '/' + photo
        + '?' + (size? 's=' + size + '&' : '')
        + (Auth.isLoggedIn() ? 'access_token=' + Auth.getToken() : '') ;
    };

    //
    // Used when loading new sessions
    scope.hasSocialActivity=function(session,field){
      if(!scope.boat[field]||!scope.boat[field].length){
        return false;
      }
      var start=new Date(session.startTime);
      var end=new Date(session.endTime);

      return (scope.boat[field].filter(function(elem) {
        var when=new Date(elem.when)
        return start<=when&&end>=when;
      }).length>0);

    };

    scope.twdirToCardinal = function(twdir) {
      if (typeof(twdir) != 'number') {
        return '-';
      }

      var index = Math.round(360 + twdir * 8 / 360) % 8;
      var windrose = [
        "N", "NE", "E", "SE", "S", "SW", "W", "NW" ];
      return windrose[index];
    };

    scope.knotsToBeaufort = function(knots) {
      if (knots < 1) { return 0; }
      if (knots < 3) { return 1; }
      if (knots < 6) { return 2; }
      if (knots < 10) { return 3; }
      if (knots < 16) { return 4; }
      if (knots < 21) { return 5; }
      if (knots < 27) { return 6; }
      if (knots < 33) { return 7; }
      if (knots < 40) { return 8; }
      if (knots < 47) { return 9; }
      if (knots < 55) { return 10; }
      if (knots < 63) { return 11; }
      return '-';
    };

    scope.timeToInt = function(time) {
      return new Date(time).getTime();
    };

    scope.perfNameForSession = function() {
      return curveStartTime($stateParams.sessionId).toLocaleString();
    };

    scope.computeEsa = function() {
      scope.esaStatus = 'in-progress';
      $http.post('/api/perfstat/'
                 + $scope.boatId + '/',
         {
           name: scope.perfNameForSession(),
           urlName: urlFriendlyForm(scope.perfNameForSession()),
           start: curveStartTimeStr($stateParams.sessionId),
           end: curveEndTimeStr($stateParams.sessionId)
         })
      .then(loadPerfStat);
    };
  });



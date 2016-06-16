'use strict';

var app = angular.module('www2App')
  .directive('boatSummary', function ($rootScope, $location, $interpolate, boatList, Auth) {
    var storedPageSize={};

    return {
      templateUrl: 'app/boatSummary/boatSummary.html',
      restrict: 'E',
      scope: {
        boatId: "=",
        pageSize: "=?"
      },
      link: function (scope, element, attrs) {

        scope.currentPage = 1;
        scope.sessions = [];

        function updateSessions() {

          if(!scope.boatId) return;

          scope.pageSize=storedPageSize[scope.boatId]||2;
          //
          // be sure that boats are ready
          boatList.boats().then(function (boats) {
            scope.boat = boatList.boat(scope.boatId);
            scope.sessions = boatList.sessionsForBoat(scope.boatId);

            //
            // ensure that sessions is not empty
            scope.sessions=scope.sessions||[];
            scope.sessions.forEach(function(session) {
              session.hasPhoto=scope.hasSocialActivity(session,'photos');
              session.hasComment=scope.hasSocialActivity(session,'comments');
            })
          });

        }



        // Why those listeners?
        // scope.$on('boatList:updated', updateSessions);
        // scope.$on('boatList:sessionsUpdated', updateSessions);
        scope.$watch('boatId', updateSessions);


        //
        // display more session
        scope.showMoreSessions=function(){
          scope.pageSize+=10;
          storedPageSize[scope.boatId]=scope.pageSize;
        }

        //
        // get photo here
        // TODO make this a directive or an API available on top controller
        scope.photoUrl = function(bid,photo, size) {
          return '/api/events/photo/' + bid + '/' + photo
            + '?' + (size? 's=' + size + '&' : '') + 'access_token=' + Auth.getToken() ;
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



        //
        // get first session date
        // TODO this could be a directive 
        scope.sessionGetFirstDate=function(sessions) {
          return sessions.length&&sessions[0].startTime;
        };
        //
        // compute avg in current sessions for direction
        // TODO this could be a directive 
        scope.sessionTotalDistance=function (sessions) {
          var sum = sessions.reduce(function(prev, session) { 
            return prev + session.trajectoryLength; 
          },0);
          return sum.toFixed(1);
        };

        //
        // compute avg in current sessions for speed
        // TODO this could be a directive 
        scope.sessionAvgSpeed=function (sessions) {
          var sum = sessions.reduce(function(prev, session) { 
            return prev + session.avgWindSpeed; 
          },0);
          return (sum / sessions.length).toFixed(1);
        };

        //
        // compute avg in current sessions for speed
        // TODO this could be a directive 
        scope.sessionMaxBoatSpeed=function (sessions) {
          return Math.max.apply(Math,sessions.map(function(session){
            return session.maxSpeedOverGround;
          })).toFixed(2);
        };




        scope.twdirToCardinal = function(twdir) {
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
          return 12;
        };
      }
    };
  });


app.directive('boatMainImage', ['$parse', function($parse) {
  var style={
    'background-size':'cover',
    'background-color':'transparent',
    'background-position': '50% 20%',
    'height':'100%'
  }, defaultImage="/assets/images/boat-sample.png";    

  return {
    restrict: 'A',
    replace: false, 
    scope:{
      boatMainImage:'='
    },
    link: function(scope, element, attrs, ngModelCtrl) {
      var self=this;

      scope.$watch('boatMainImage', function (boatMainImage) {
        
        //
        // initial value
        style['background-image']='url('+defaultImage+')';
        if(boatMainImage&&
           boatMainImage._id&&
           boatMainImage.photos&&
           boatMainImage.photos.length){
          var path=scope.$parent.photoUrl(boatMainImage._id,boatMainImage.photos[0].src,'300x400');
          style['background-image']='url('+path+')';
        }
        element.css(style);
      });
    }
  }
}]);



app.filter('startFrom', function() {
    return function(input, start) {
        start = +start; //parse to int
        if (input) return input.slice(start);
    }
});

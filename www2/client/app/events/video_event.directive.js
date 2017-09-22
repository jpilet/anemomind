'use strict';

angular.module('www2App')
  .directive('videoEvent', function ($timeout) {
    return {
      templateUrl: 'app/events/video_event.html',
      restrict: 'E',
      scope: {
        event: '=',
        currentTime: '=',
      },
      link: function (scope, element, attrs) {
	$timeout(function() {
	  scope.sources = {
	      sources: [
		  {
		      src: '/videos/' + scope.event.video + '-s.mp4',
		      type: 'video/mp4'
		  }
	      ]
	  };
	}, 10);

	var start = scope.event.when.getTime();
	var end = new Date(scope.event.videoEnd).getTime();
	var player;

	scope.$on('vjsVideoReady', function(id, vid, _, controlbar) {
	  player = vid.player;
	  vid.player.on('timeupdate', function(timeUpdateEvent) {
	    scope.$apply(function() {
	      var ratio =  player.currentTime() / player.duration();
	      var newTime = start + ratio * (end - start);
	      if (Math.abs(newTime - scope.currentTime.getTime()) > 500) {
		scope.currentTime.setTime(newTime);
	      }
	    });
	  });
	});
	scope.$watch('currentTime', function(newTime, oldTime) {
	  if (!player) {
	    return;
	  }

	  var deltaSec = (newTime.getTime() - start) / 1000;
	  if (deltaSec > player.duration()) {
	    if (!player.paused()) {
	      player.pause();
	      player.currentTime(player.duration());
	    }
	  } else if (deltaSec < 0) {
	    if (!player.paused()) {
	      player.pause();
	      player.currentTime(0);
	    }
	  } else {
	    if (!oldTime) {
	      player.currentTime(deltaSec);
	    } else {
	      // if this event comes from the player itself, we need to ignore
	      // it. Otherwise, we need to honor it.
	      var update = Math.abs(deltaSec - player.currentTime());
	      if (update > 1) {
		player.currentTime(deltaSec);
	      }
	    }
	  }
	}, true);
      }
    };
  });

'use strict';

angular.module('www2App')
  .filter('timeSpan', function () {
      return function(span) {
        var durSeconds;
        if (isFinite(span)) {
          durSeconds = span;
        } else if (span == undefined) {
          return '?';
        } else {
          var fromDate = new Date(span[0]);
          var toDate = new Date(span[1]);

          durSeconds = 0.001*(toDate.getTime() - fromDate.getTime());
        }

        var dur = durSeconds;
        var multipliers = [60, 60, 24, 7, 1000000000];
        var labels = ['second', 'minute', 'hour', 'day', 'week'];
        var formatDur = function(x, s) {
          return (x == 0? null : x + ' ' + s + (x == 1? "" : "s"));
        }
        var dst = [];
        var precedingGap = 0;
        for (var i = 0; i < multipliers.length; i++) {
          var f = multipliers[i];
          var a = Math.floor(dur/f);
          var b = Math.floor(dur - f*a);
          var s = formatDur(b, labels[i]);
          if (s) {
                  dst.push({value: s, gap: precedingGap});
            precedingGap = 0;
          } else {
            precedingGap++;
          }
          dur = a;
        }
        var finalParts = dst.reverse();
        if (finalParts.length == 0) {
          return "0 seconds";
        } else {
          var first = finalParts[0];
          if (finalParts.length == 1 || first.gap > 0) {
            return first.value;
          } else {
            return first.value + ', ' + finalParts[1].value;
          }
        }
      };
  });

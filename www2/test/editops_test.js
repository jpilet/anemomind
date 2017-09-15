var SessionOps = require('../client/app/SessionOps.js');
var anemoutils = require('../client/app/anemoutils.js');
var assert = require('assert');

// From Courdlineone
var rawSessions = [{"_id":"59baa28347ef69df80cbbe932017-04-06T11:35:552017-04-06T19:13:09","boat":"59baa28347ef69df80cbbe93","trajectoryLength":40.044511504230776,"maxSpeedOverGround":8.955417045479273,"maxSpeedOverGroundTime":"2017-04-06T11:36:43.451Z","startTime":"2017-04-06T11:35:55.451Z","endTime":"2017-04-06T19:13:09.878Z","location":{"x":0.32410883335857626,"y":0.4488435014439503,"scale":0.002573157912979185},"avgWindSpeed":16.13048475920258,"avgWindDir":87.25538063748184,"strongestWindSpeed":21.803744719826028,"strongestWindTime":"2017-04-06T12:41:28.451Z","hasPhoto":false,"hasComment":false},{"_id":"59baa28347ef69df80cbbe932017-04-08T12:09:102017-04-08T16:36:23","boat":"59baa28347ef69df80cbbe93","trajectoryLength":20.51894778853303,"maxSpeedOverGround":7.490605387445659,"maxSpeedOverGroundTime":"2017-04-08T12:38:49.467Z","startTime":"2017-04-08T12:09:10.467Z","endTime":"2017-04-08T16:36:23.290Z","location":{"x":0.32506796066794,"y":0.44925134348351053,"scale":0.0013940988422123102},"avgWindSpeed":12.778396228118647,"avgWindDir":110.63178447745692,"strongestWindSpeed":18.653570492612165,"strongestWindTime":"2017-04-08T14:53:53.290Z","hasPhoto":false,"hasComment":false},{"_id":"59baa28347ef69df80cbbe932017-04-08T21:41:182017-04-08T21:44:08","boat":"59baa28347ef69df80cbbe93","trajectoryLength":0.05649168192388169,"maxSpeedOverGround":2.0900824211106706,"maxSpeedOverGroundTime":"2017-04-08T21:42:23.301Z","startTime":"2017-04-08T21:41:18.301Z","endTime":"2017-04-08T21:44:08.301Z","location":{"x":0.32541709366852345,"y":0.4494555163197358,"scale":0.0000017863917176619282},"avgWindSpeed":3.3399999999999994,"avgWindDir":171.70132802070856,"strongestWindSpeed":3.34,"strongestWindTime":"2017-04-08T21:44:08.301Z"},{"_id":"59baa28347ef69df80cbbe932017-04-09T14:12:422017-04-09T19:55:41","boat":"59baa28347ef69df80cbbe93","trajectoryLength":39.721160362367954,"maxSpeedOverGround":14.29029077217424,"maxSpeedOverGroundTime":"2017-04-09T18:13:31.780Z","startTime":"2017-04-09T14:12:42.776Z","endTime":"2017-04-09T19:55:41.780Z","location":{"x":0.32535098629026976,"y":0.4494969651799683,"scale":0.0004612313498818521},"avgWindSpeed":12.10728399950813,"avgWindDir":120.09229021757255,"strongestWindSpeed":17.258961920960168,"strongestWindTime":"2017-04-09T18:02:45.780Z","hasPhoto":false,"hasComment":false},{"_id":"59baa28347ef69df80cbbe932017-04-10T00:03:092017-04-10T00:04:36","boat":"59baa28347ef69df80cbbe93","trajectoryLength":0.021503256516935566,"maxSpeedOverGround":0.1983019407597349,"maxSpeedOverGroundTime":"2017-04-10T00:03:52.563Z","startTime":"2017-04-10T00:03:09.563Z","endTime":"2017-04-10T00:04:36.563Z","location":{"x":0.3254166389567092,"y":0.4494555716801897,"scale":1.853068252177792e-7},"avgWindSpeed":10.01,"avgWindDir":155.00359615670857},{"_id":"59baa28347ef69df80cbbe932017-04-10T14:36:172017-04-10T18:52:59","boat":"59baa28347ef69df80cbbe93","trajectoryLength":31.59318791280491,"maxSpeedOverGround":13.21649623751645,"maxSpeedOverGroundTime":"2017-04-10T15:36:25.183Z","startTime":"2017-04-10T14:36:17.183Z","endTime":"2017-04-10T18:52:59.728Z","location":{"x":0.3253190133433433,"y":0.4493374714261621,"scale":0.00049404288105237},"avgWindSpeed":13.359511006625004,"avgWindDir":111.47797707770734,"strongestWindSpeed":18.580810872573434,"strongestWindTime":"2017-04-10T18:05:54.728Z","hasPhoto":false,"hasComment":false,"$$hashKey":"object:29"},{"_id":"59baa28347ef69df80cbbe932017-04-11T13:07:082017-04-11T20:11:54","boat":"59baa28347ef69df80cbbe93","trajectoryLength":53.2076942666723,"maxSpeedOverGround":12.84478581476209,"maxSpeedOverGroundTime":"2017-04-11T18:21:08.471Z","startTime":"2017-04-11T13:07:08.677Z","endTime":"2017-04-11T20:11:54.471Z","location":{"x":0.3253285059295166,"y":0.44939811475180047,"scale":0.0007770875566426305},"avgWindSpeed":12.185535547682539,"avgWindDir":122.11469017654079,"strongestWindSpeed":16.92431972262417,"strongestWindTime":"2017-04-11T17:35:46.132Z","hasPhoto":false,"hasComment":false,"$$hashKey":"object:28"}];

function stripIrrelevant(x) {
  return {
    startTime: x.startTime, 
    endTime: x.endTime
  };
}

describe('Edit ops', function() {
  console.log("Number of sessions: %d", rawSessions.length);

  it('Delete operation', function() {
    var sessions = rawSessions.map(SessionOps.normalizeSession);
    
    var tree = SessionOps.buildSessionTree(sessions.map(stripIrrelevant));
    console.log(JSON.stringify(tree, null, 4));
    assert(tree.startTime == sessions[0].startTime);
    assert(tree.endTime + '' == sessions[sessions.length-1].endTime);

    var countFun = function(dst, x) {
      return dst + 1;
    };
    var leafCount = SessionOps.reduceSessionTreeLeaves(countFun, 0, tree);

    assert(sessions.length == leafCount);

    var addDur = anemoutils.map(SessionOps.sessionDurationSeconds)(anemoutils.add);
    var totalDuration = SessionOps.reduceSessionTreeLeaves(addDur, 0, tree);

    console.log("Total duration is " + totalDuration/3600 + " hours");

    var index = 3;
    var sessionToDelete = sessions[index];
    var durationToDelete = SessionOps.sessionDurationSeconds(sessionToDelete);

    var tree2 = SessionOps.applyEdit(tree, {
      type: "delete",
      lower: sessionToDelete.startTime,
      upper: sessionToDelete.endTime
    });

    var totalDuration2 = SessionOps.reduceSessionTreeLeaves(addDur, 0, tree2);
    console.log("New duration: " + totalDuration2/3600 + " hours");
    assert(Math.abs((totalDuration - totalDuration2) - durationToDelete) < 1.0e3);

    var leafCount2 = SessionOps.reduceSessionTreeLeaves(countFun, 0, tree2);
    
    


  });
});

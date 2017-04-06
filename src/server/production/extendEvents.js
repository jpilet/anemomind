
function boatInfoAtTime(boat, time) {
  var query = {
    $and: [
      {boat: ObjectId(boat + '') },
      {startTime: {$lte: time, $gte: new Date(time.getTime() - 60 * 60 * 1000)} },
      {endTime: {$gt: time} }
    ]
  };

  var closest;
  db.tiles.find(query).forEach(function(tile) {
     tile.curves.forEach(function(c) {
       c.points.forEach(function(p) {
         if (closest == undefined
             || Math.abs(p.time - time) < Math.abs(closest.time - time)) {
           closest = p;
         }
       });
     });
   });
  return closest;
};

var query = {};

if (boatid) {
  query.boat = boatid;
}

if (onlynew) {
  query.dataAtEventTime = null;
}

db.events.find(query).forEach(function(ev) {
  var info = boatInfoAtTime(ev.boat, ev.when);
  if (info) {
    var delta = Math.abs(ev.when.getTime() - info.time.getTime()) / 1000;
    if (delta < 10) {
      db.events.update({"_id": ev._id}, {$set: { dataAtEventTime: info }});
    } else {
      print('delta too large. Event: ' + ev._id
            + ' boat: ' + ev.boat
            + ' event time: ' + ev.when
            + ' delta time: ' + delta);
    }
  } else {
    print('no data. Event: ' + ev._id
          + ' boat: ' + ev.boat
          + ' time: ' + ev.when);
  }
});

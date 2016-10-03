
function curveEndTimeStr(curveId) {
  return curveId.substr(curveId.length-19);
}

function curveStartTimeStr(curveId) {
  return curveId.substr(curveId.length-19*2,19);
}

function curveEndTime(curveId) {
  // Adding a Z sets the time zone to GMT.
  return new Date(curveEndTimeStr(curveId) + "Z");
}

function curveStartTime(curveId) {
  return new Date(curveStartTimeStr(curveId) + "Z");
}

function zeroPad2(nr){
  var  len = (2 - String(nr).length)+1;
  return len > 0? new Array(len).join('0')+nr : nr;
}

function encodeTime(time) {
  return time.getUTCFullYear() + '-'
    + zeroPad2(time.getUTCMonth() + 1) + '-'
    + zeroPad2(time.getUTCDate())
    + 'T' + zeroPad2(time.getUTCHours())
    + ':' + zeroPad2(time.getUTCMinutes())
    + ':' + zeroPad2(time.getUTCSeconds());
}

function makeCurveId(boat, startTime, endTime) {
  return boat + encodeTime(startTime) + encodeTime(endTime);
}

function VectorTileLayer(params, renderer) {
  this.params = params;

  this.renderer = renderer;

  this.boatIcon = new Image();
  this.boatIcon.src = '/assets/images/boat.svg';
  this.trueWindIcon = new Image();
  this.trueWindIcon.src = '/assets/images/truewind.svg';
  this.appWindIcon = new Image();
  this.appWindIcon.src = '/assets/images/appwind.svg';

  this.tiles = {};
  this.numLoading = 0;
  this.loadQueue = [];
  this.numDraw = 0;
  this.numCachedTiles = 0;
  this.visibleCurves = {};
  this.curvesFlat = {};
  this.queueSeconds = 0;
  this.currentTime = 0;

  this.currentColor = '';
  this.startPoint = 0;
  this.origStrokeStyle = '';
  this.origlineWidth = 0;
  this.tailWidth = 3;
  this.vmgPerf = [0, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110];
  this.colorSpectrum = ['#B5B5B5','#BAA7AA','#C099A0','#C68B96','#CC7D8C',
                        '#D26F81','#D86277','#DE546D','#E44663','#EA3858',
                        '#F02A4E','#F61C44','#FC0F3A','#DE0B66','#C10792',
                        '#A403BF'];

  if (!("tileSize" in this.params)) this.params.tileSize = 256;
  if ("vectorurl" in this.params) {
    this.url = this.params.vectorurl;
  } else {
    this.url = undefined;
  }
  if (params.debug) {
      this.debug = function(msg) { console.log(msg); }
  } else {
      this.debug = function(msg) { };
  }
  if (!this.params.maxNumCachedTiles) this.params.maxNumCachedTiles = 64;
  if (!this.params.maxSimultaneousLoads) this.params.maxSimultaneousLoads = 3;


}

VectorTileLayer.prototype.setUrl = function(url) {
  if (this.url == url) {
    return;
  }
  this.url = url;

  // URL change: let's reload everything.
  this.tiles = {};
  this.loadQueue = [];
  this.visibleCurves = {};

  this.renderer.refreshIfNotMoving();
};

VectorTileLayer.prototype.draw = function(canvas, pinchZoom,
                                    bboxTopLeft, bboxBottomRight) {
  if (!this.url) {
    return;
  }

  // Compute the scale level
  var numTiles = canvas.width / this.params.tileSize;
  var targetUnitPerTile = (bboxBottomRight.x - bboxTopLeft.x) / numTiles;
  var scale = Math.max(0, Math.ceil(- Math.log(targetUnitPerTile) / Math.LN2));
  var actualUnitPerTile = 1 / (1 << scale);

  var getTileX = function(unitX) { return  Math.floor(unitX * (1 << scale)); };
  var getTileY = function(unitY) { return  getTileX(unitY); };
  
  var firstTileX = getTileX(Math.max(0, bboxTopLeft.x));
  var firstTileY = getTileY(Math.max(0, bboxTopLeft.y));
  var lastTileX = getTileX(Math.min(this.renderer.params.width, bboxBottomRight.x));
  var lastTileY = getTileY(Math.min(this.renderer.params.height, bboxBottomRight.y));

  Utils.assert(firstTileY != undefined);

  var zoom = 1.0 / (1 << scale);
  var tileGeometry = {
    origin: pinchZoom.viewerPosFromWorldPos(firstTileX * zoom,
                                            firstTileY * zoom),
    delta: pinchZoom.viewerPosFromWorldPos((firstTileX + 1) * zoom,
                                           (firstTileY + 1) * zoom),
    firstTileX: firstTileX,
    firstTileY: firstTileY
  };
  // We address canvas pixels in integer coordinates to avoid
  // inconsistencies across browsers.
  tileGeometry.delta.x = Math.round(tileGeometry.delta.x - tileGeometry.origin.x);
  tileGeometry.delta.y = Math.round(tileGeometry.delta.y - tileGeometry.origin.y);
  tileGeometry.origin.x = Math.round(tileGeometry.origin.x);
  tileGeometry.origin.y = Math.round(tileGeometry.origin.y);

  var context = canvas.getContext('2d');

  this.visibleCurves = {};

  for (var tileY = firstTileY; tileY <= lastTileY; ++tileY) {
    for (var tileX = firstTileX; tileX <= lastTileX; ++tileX) {
      this.requestTile(scale, tileX, tileY, tileGeometry, canvas);
    }
  }
  this.drawVisibleCurves(context, pinchZoom);
  this.drawTimeSelection(context, pinchZoom);

  this.processQueue();

  // control memory usage.
  this.limitCacheSize();

  this.numDraw++;
}

VectorTileLayer.prototype.requestTile = function(scale, tileX, tileY,
                                                tileGeometry, canvas) {
  var left = tileGeometry.origin.x
      + tileGeometry.delta.x * (tileX - tileGeometry.firstTileX);
  var top = tileGeometry.origin.y
      + tileGeometry.delta.y * (tileY - tileGeometry.firstTileY);

  if (left >= canvas.width || top >= canvas.height) {
    return;
  }

  for (var upLevel = 0; upLevel <= scale && upLevel < 4; ++upLevel) {
    var upTileX = tileX >> upLevel;
    var upTileY = tileY >> upLevel;
    
    var tile = this.getTile(scale - upLevel, upTileX , upTileY, 1 - upLevel * .15);
    if (tile && tile.state == "loaded") {

      for (var curve in tile.data) {

        for (var c in tile.data[curve].curves) {
          var curveId = tile.data[curve].curves[c].curveId;
          var curveData = tile.data[curve].curves[c];

          if (!this.visibleCurves[curveId]) {
            this.visibleCurves[curveId] = { length: 0 };
          }
          var key = tile.key + c;
          if (!(key in this.visibleCurves[curveId])) {
            ++this.visibleCurves[curveId].length;
          }
          this.visibleCurves[curveId][key] = curveData;
        }
      }
      return;
    }
  }
};

// If selectedCurve does not match exactly the start and end times of a
// recorded session, we still want to display part of it.
// This function returns true if both curve times overlap.
function curveOverlap(a, b) {
  if (a == b) {
    return true;
  }

  var AendTime = curveEndTime(a);
  var AstartTime = curveStartTime(a);
  var BendTime = curveEndTime(b);
  var BstartTime = curveStartTime(b);

  return ((BendTime > AstartTime) && (BstartTime < AendTime));
}

VectorTileLayer.prototype.drawVisibleCurves = function(context, pinchZoom) {
  if (this.selectedCurve) {
    for (var curveId in this.visibleCurves) {
      if (curveOverlap(curveId, this.selectedCurve)) {
        this.drawCurve(curveId, context, pinchZoom);
      }
    }
  } else {
    for (var curveId in this.visibleCurves) {
      this.drawCurve(curveId, context, pinchZoom);
    }
  }
}

function byTime(a, b) {
  return a.time - b.time;
}

/**
 * Performs a binary search on the provided sorted list and returns the index
 * of the item if found. If it can't be found it'll return -1.
 * Inspired from https://github.com/Wolfy87/binary-search
 *
 * @param {*[]} list Items to search through.
 * @param {*} item The item to look for.
 * @return {Number} The index of the item if found, -1 if not.
 */
var binarySearch = function(list, item) {
  var min = 0;
  var max = list.length - 1;
  var guess;

  while ((max - min) > 1) {
      guess = Math.floor((min + max) / 2);

      if (list[guess].time < item) {
          min = guess;
      }
      else {
          max = guess;
      }
  }

  return [min, max];
};

var selectByTime = function(points, start, end) {
  if (end < points[0].time || start > points[points.length - 1].time) {
    return [];
  }
  var startBounds = binarySearch(points, start);
  var endBounds = binarySearch(points, end);
  return points.slice(startBounds[0], endBounds[1]);
};

VectorTileLayer.prototype.getPointsForCurve = function(curveId) {
  // Extract all points
  var curveElements = this.visibleCurves[curveId];
  if (!curveElements) {
    // search for an overlaping curve.
    for (var curve in this.visibleCurves) {
      if (curveOverlap(curve, curveId)) {
        curveElements = this.visibleCurves[curve];
        break;
      }
    }
    if (!curveElements) {
      return [];
    }
  }

  var len = curveElements.length;

  if (curveId in this.curvesFlat && this.curvesFlat[curveId].length == len) {
    this.lastPointArray = this.curvesFlat[curveId].points;
    return this.lastPointArray;
  }

  var elementsAsArray = [];
  for (var e in curveElements) {
    if (e != "length") {
      var element = curveElements[e];
      if (curveOverlap(element.curveId, curveId)) {
        elementsAsArray.push(element.points);
      }
    }
  }

  var points = Array.prototype.concat.apply([], elementsAsArray);

  // Sort by time
  points.sort(byTime);

  if (this.selectedCurve && this.selectedCurve != curveId) {
    var start = curveStartTime(this.selectedCurve);
    var end = curveEndTime(this.selectedCurve);
    points = selectByTime(points, start, end);
  }

  this.lastPointArray = points;

  this.curvesFlat[curveId] = { length: len, points: points };

  return points;
}


VectorTileLayer.checkIfTail = function(currentTime, queueSeconds, context, point) {
  var queueTime = Math.abs(currentTime.getTime() - (queueSeconds * 1000));
  var pointTime = point.time;

  if(pointTime.getTime() >= queueTime && pointTime.getTime() <= currentTime.getTime()) {
    return true;
  }
  return false;
}

VectorTileLayer.strokePath = function(context) {
  context.stroke();
  context.beginPath();
}

VectorTileLayer.prototype.createGradient = function(context, color, startPoint, endPoint) {
  var index = this.colorSpectrum.indexOf(color);
  var nextColor = '';
  var grd = null;

  if(typeof this.colorSpectrum[index + 1] == 'undefined')
    nextColor = this.colorSpectrum[0];
  else
    nextColor = this.colorSpectrum[index + 1];
  
  if(this.currentColor != color && this.currentColor != '' && startPoint != 0) {
    grd = context.createLinearGradient(startPoint.x, startPoint.y, endPoint.x, endPoint.y);
    grd.addColorStop(0, this.currentColor);
    grd.addColorStop(1, color);
  }

  return grd;
};

VectorTileLayer.prototype.vmgPerfOfTail = function(context, startOfTail, currentLineSegment, currentPoint, previousPoint) {
  for(var i=0; i<this.vmgPerf.length; i++) {
    if(this.vmgPerf[i] != this.vmgPerf[this.vmgPerf.length - 1]) {
      if(perfAtPoint(currentLineSegment) > this.vmgPerf[i] && perfAtPoint(currentLineSegment) <= this.vmgPerf[i+1]) {
        if(!startOfTail) {
          this.startPoint = previousPoint;
          this.currentColor = this.colorSpectrum[i];
          return;
        }
        
        if(this.currentColor != this.colorSpectrum[i]) {
          var grd = this.createGradient(context, this.colorSpectrum[i], this.startPoint, currentPoint);
          if(grd === null) {
            context.strokeStyle = this.origStrokeStyle;
            context.lineWidth = this.origlineWidth;
          }
          else {
            context.strokeStyle = grd;
            context.lineWidth = this.tailWidth;
          }            
          VectorTileLayer.strokePath(context);
          context.moveTo(previousPoint.x, previousPoint.y);
          this.startPoint = previousPoint;
          this.currentColor = this.colorSpectrum[i];
        }

        return;
      }
    }
    else {
      if(perfAtPoint(currentLineSegment) > this.vmgPerf[i]) {
        if(!startOfTail) {
          this.startPoint = previousPoint;
          this.currentColor = this.colorSpectrum[i];

          return;
        }
        if(this.currentColor != this.colorSpectrum[i]) {
          var grd = this.createGradient(context, this.colorSpectrum[i], this.startPoint, currentPoint);
          if(grd === null) {
            context.strokeStyle = this.origStrokeStyle;
            context.lineWidth = this.origlineWidth;
          }
          else {
            context.strokeStyle = grd;
            context.lineWidth = this.tailWidth;
          }
          VectorTileLayer.strokePath(context);
          context.moveTo(previousPoint.x, previousPoint.y);

          this.startPoint = previousPoint;
          this.currentColor = this.colorSpectrum[i];
        }

        return;
      }
    }
  }
}

VectorTileLayer.prototype.drawCurve = function(curveId, context, pinchZoom) {
  // prepare the Cavas path

  if (this.isHighlighted(curveId)) {
    context.strokeStyle="#9E9E9E";
    context.lineWidth = 1;
  } else {
    if (this.highlight) {
      // Another curve is hightlighted
      context.strokeStyle="#777777";
      context.lineWidth = 1;
    } else {
      // Nothing is highlighted
      context.strokeStyle="#333333";
      context.lineWidth = 2;
    }
  }
  
  this.origStrokeStyle = context.strokeStyle;
  this.origlineWidth = context.lineWidth;
  var startOfTail = false;
  var vLayer = this;

  var points = this.getPointsForCurve(curveId);
  if (points.length == 0) {
    return;
  }

  var createTailTrack = function(currentLineSegment, currentPoint, previousPoint) {
    if(!startOfTail) {
      VectorTileLayer.strokePath(context);
      context.moveTo(previousPoint.x, previousPoint.y);
      context.lineTo(currentPoint.x, currentPoint.y);
      vLayer.vmgPerfOfTail(context, startOfTail, currentLineSegment, currentPoint, previousPoint);
      startOfTail = true;

      return;
    }
    context.lineTo(previousPoint.x, previousPoint.y);
    vLayer.vmgPerfOfTail(context, startOfTail, currentLineSegment, currentPoint, previousPoint);

    return;
  };

  // Draw.
  context.beginPath();
  var first = pinchZoom.viewerPosFromWorldPos(points[0].pos[0],
                                              points[0].pos[1]);
  context.moveTo(first.x, first.y);

  for (var i = 1; i < points.length; ++i) {
    var currentPoint = pinchZoom.viewerPosFromWorldPos(points[i].pos[0],
                                                points[i].pos[1]);

    var previousPoint = pinchZoom.viewerPosFromWorldPos(points[i-1].pos[0],
                                                points[i-1].pos[1]);

    if(this.queueSeconds > 0 && currentPoint)
      var tail = VectorTileLayer.checkIfTail(this.currentTime, this.queueSeconds, context, points[i]);

    if(tail) {
      createTailTrack(points[i], currentPoint, previousPoint);
    }
    else {
      if(context.strokeStyle != vLayer.origStrokeStyle) {
        context.moveTo(previousPoint.x, previousPoint.y);
        VectorTileLayer.strokePath(context);
        context.strokeStyle = vLayer.origStrokeStyle;
        context.lineWidth = vLayer.origlineWidth;
        context.lineTo(previousPoint.x, previousPoint.y);
      }
      else {
        context.lineTo(currentPoint.x, currentPoint.y);
      }
    }
    
  }
  context.stroke();
  /*
  console.log('Curve ' + curveId + ' span: ' + points[0].time + ' to '
              + points[points.length - 1].time);
  */
}

VectorTileLayer.prototype.setCurrentTime = function(time) {
  this.currentTime = time;
  this.renderer.refreshIfNotMoving();
}
  
VectorTileLayer.prototype.drawTimeSelection = function(context, pinchZoom) {
  if (!this.currentTime || !this.lastPointArray ||
      this.lastPointArray.length < 2 ||
      this.currentTime < this.lastPointArray[0] ||
      this.currentTime > this.lastPointArray[this.lastPointArray.length - 1]) {
    return;
  }

  var pixelRatio = this.renderer.pixelRatio;
  var circleRadius = 45 * pixelRatio;

  var rotateAndDrawIcon = function(angle, icon) {
    if (angle == undefined) {
      return;
    }
    context.save();
    context.rotate(angle * toRadians);

    var d = circleRadius;
    var l = icon.height * pixelRatio;
    var w = icon.width * pixelRatio;
    context.drawImage(icon, -w/2, - d - l/2, w, l);
    context.restore();
  }

  var windArrow = function(angle, color) {
    if (angle == undefined) {
      return;
    }
    var d = 20 * pixelRatio;
    var l = 30 * pixelRatio;
    var w = 8 * pixelRatio;
    context.save();
    context.rotate((180+angle) * toRadians);
    context.beginPath();
    context.moveTo(-w/2, d + l);
    context.lineTo(w/2, d + l);
    context.lineTo(0, d);
    context.closePath();
    context.fillStyle = color;
    context.fill();
    context.restore();
  }

  var getTwdir = function(nav) {
    if (nav.twdir) {
      return nav.twdir;
    }
    if (nav.deviceTwdir) {
      return nav.deviceTwdir;
    }
    if (nav.externalTwa) {
      return nav.externalTwa + nav.gpsBearing;
    }
    return undefined;
  };


  var bounds = binarySearch(this.lastPointArray, this.currentTime);
  var p = this.lastPointArray[bounds[0]];
  var pos = pinchZoom.viewerPosFromWorldPos(p.pos[0], p.pos[1]);

  var toRadians = Math.PI / 180.0;
  context.save();
  context.translate(pos.x, pos.y);

  
  context.save();
  context.rotate(p.gpsBearing * toRadians);

  if (this.boatIcon.complete) {
    var r = circleRadius * 1.1;
    var n = 32;
    context.beginPath();
    for (var i = 0; i < n; ++i) {
      var angle = (i / n) * 2 * Math.PI;
      var t = ((i % 4) == 0 ? 10 : 6) * pixelRatio;
      var cos = Math.cos(angle);
      var sin = Math.sin(angle);
      context.moveTo(cos * r, sin * r);
      context.lineTo(cos * (r - t), sin * (r - t));
    }
    context.strokeStyle = '#ff0033';
    context.stroke();

    var l = 38 * pixelRatio;
    var w = 25 * pixelRatio;
    context.drawImage(this.boatIcon,
                      - w/2,
                      - l/2,
                      w, l);
  } else {
    // Icon not loaded yet. Fall back on a rough triangle.
    context.beginPath();
    context.moveTo(0, -l/2);
    context.lineTo(w/2, l/2);
    context.lineTo(-w/2, l/2);
    context.closePath();
    context.fillStyle = '#662200';
    context.fill();
  }

  if (this.appWindIcon.complete) {
    rotateAndDrawIcon(p.awa, this.appWindIcon);
  } else {
    windArrow(p.awa, '#774400');
  }

  context.restore();

  var twdir = getTwdir(p);
  if (this.trueWindIcon.complete) {
    rotateAndDrawIcon(twdir, this.trueWindIcon);
  } else {
    windArrow(twdir, '#7744ff');
  }

  context.restore();

}

VectorTileLayer.prototype.getTile = function(scale, x, y, priority) {
  var key = scale + "," + x + "," + y;
  
  if (key in this.tiles) {
    var tile = this.tiles[key];
    if (tile.lastDrawRequest == this.numDraw) {
      tile.priority += priority;
    } else {
      tile.lastDrawRequest = this.numDraw;
      tile.priority = priority;
    }
    return tile;
  }
  
  if (typeof(this.url) == "function") {
    var url = this.url(scale, x, y);
  } else {
    var url = this.url
      .replace("$x", '' + x)
      .replace("$y", '' + y)
      .replace("$scale", '' + scale);
  }
  return this.queueTileRequest(key, url, priority);
};

VectorTileLayer.prototype.queueTileRequest = function(key, url, priority) {
  var tile = { lastDrawRequest: this.numDraw, priority: priority, state: "queue" };
  Utils.assert(tile.priority != undefined);
  this.loadQueue.push({key: key, url: url, tile: tile});
  this.tiles[key] = tile;
  tile.key = key;
  return tile;
};

// Convert string time representation to javascript Date() objects.
function parseTime(data) {
  for (var i in data) {
    var curves = data[i].curves;
    for (var c in curves) {
      var curve = curves[c];
      for (var p in curve.points) {
        curve.points[p].time = new Date(curve.points[p].time);
      }
    }
  }
}

VectorTileLayer.prototype.processQueue = function() {
  var queue = this.loadQueue;
  
  // Prioritize loading
  if (this.numLoading < this.params.maxSimultaneousLoads && queue.length > 0) {
    this.loadQueue.sort(function(a, b) {
      if (a.tile.lastDrawRequest == b.tile.lastDrawRequest) {
        return a.tile.priority - b.tile.priority;
      }
      return a.tile.lastDrawRequest - b.tile.lastDrawRequest;
    });
  }
  while (this.numLoading < this.params.maxSimultaneousLoads && queue.length > 0) {  
    var query = this.loadQueue.pop();
    
    // Check if the tile is still required.
    if ((this.numDraw - query.tile.lastDrawRequest) < 3) {
      this.numLoading++;
      query.tile.state = "loading";
      
      // Force the creation of a new scope to make sure
      // a new closure is created for every "query" object. 
      var f = (function(t, query) {
        $.ajax({
          url: query.url,
          dataType: "json",
          success: function(data) {
            t.numLoading--;
            t.numCachedTiles++;
            if (data.length > 0) {
              query.tile.state = "loaded";
              parseTime(data);
              query.tile.data = data;
              t.renderer.refreshIfNotMoving();
              if (t.onDataArrived) {
                if (t.onDataArrivedTimer) {
                  clearTimeout(t.onDataArrivedTimer);
                }
                t.onDataArrivedTimer = setTimeout(function() {
                  t.onDataArrivedTimer = undefined;
                  t.onDataArrived();
                }, 100);
              }
            } else {
              query.tile.state = "empty";
            }
          },
          error: function() {
            t.numLoading--;
            query.tile.state = "failed";
            console.log('Failed to load: ' + query.url);
            t.processQueue();
          },
          timeout: 2000,
          beforeSend: function(xhr) {
            if (t.params.token) {
              xhr.setRequestHeader('Authorization','Bearer ' + t.params.token);
            }
          }
        });  
      })(this, query);
      
    } else {
      // There's no need to load this tile, it is not required anymore.
      delete this.tiles[query.key];
    }
  }
};

VectorTileLayer.prototype.limitCacheSize = function() {
  if (this.numCachedTiles <= this.params.maxNumCachedTiles) {
    // The cache is small enough.
    return;
  }

  // Build an array of tiles we may need to remove from cache  
  var cache = [];
  for (var key in this.tiles) {
    var tile = this.tiles[key];
    // We do not remove tiles that are currently displayed.
    if (tile.image && tile.lastDrawRequest != this.numDraw) {
      cache.push(key);
    }
  }
  
  // Sort it: oldest request first.
  var t = this;  
  cache.sort(function(a,b) { return t.tiles[a].lastDrawRequest - t.tiles[b].lastDrawRequest; });
  
  // Remove old tiles.
  var numToRemove = cache.length - this.params.maxNumCachedTiles;
  for (var i = 0; i < numToRemove; ++i) {
    var key = cache[i];
    delete this.tiles[key];
    this.numCachedTiles--;
  }
};

VectorTileLayer.prototype.highlightCurve = function(curveId) {
  return {
    curveId: curveId,
    endTime: curveEndTime(curveId),
    startTime: curveStartTime(curveId)
  };
};

VectorTileLayer.prototype.getTimeData = function() {
  var result = [];
  for (var c in this.visibleCurves) {
    result.push(this.highlightCurve(c));
  }
  return result;
};

VectorTileLayer.prototype.selectCurve = function(curveId) {
  if (curveId == this.selectedCurve) {
    return;
  }

  this.selectedCurve = curveId;

  if (curveId) {
    this.highlight = this.highlightCurve(curveId);

    if (this.onSelect) {
      this.onSelect(curveId);
    }
  } else {
    this.highlight = undefined;
    this.renderer.refreshIfNotMoving();
  }
}

VectorTileLayer.prototype.setHighlight = function(d) {
  if (this.selectedCurve && (!d || this.selectedCurve != d.curveId)) {
    // When a curve is selected, we refuse to highlight another one.
    // We also refuse to unhighlight.
    return;
  }
  this.highlight = d;
  this.renderer.refreshIfNotMoving();
};

VectorTileLayer.prototype.isHighlighted = function(curveId) {
  if (!this.highlight) {
    return false;
  }
  var endTime = curveEndTime(curveId);
  var startTime= curveStartTime(curveId);
  return this.highlight.startTime >= startTime
    && this.highlight.endTime <= endTime;
};

VectorTileLayer.prototype._closestPointInTile = function(x, y, key) {
  if (!(key in this.tiles)) {
    return undefined;
  }

  var tile = this.tiles[key];

  if (tile.state != 'loaded') {
    return undefined;
  }

  if (this.selectedCurve) {
    var selectedTimeStart = curveStartTime(this.selectedCurve);
    var selectedTimeEnd = curveEndTime(this.selectedCurve);
  }

  var bestDist = 1e8;
  var bestPoint = false;
  var bestCurve;
  var p = {x: x, y: y};


  for (var curve in tile.data) {
    for (var c in tile.data[curve].curves) {
      var curveId = tile.data[curve].curves[c].curveId;
      if (this.selectedCurve && !curveOverlap(this.selectedCurve, curveId)) {
        continue;
      }
        var points = tile.data[curve].curves[c].points;
      for (var i in points) {
        if (this.selectedCurve
            && (points[i].time < selectedTimeStart
                || points[i].time > selectedTimeEnd)) {
          continue;
        }
        var dist = Utils.distance(p, {x: points[i].pos[0], y: points[i].pos[1]});
        if (dist < bestDist) {
          bestDist = dist;
          bestPoint = points[i];
          bestCurve = curveId;
        }
      }
    }
  }
  if (bestPoint) {
    return { point: bestPoint, curveId: bestCurve, dist: bestDist };
  }
}

VectorTileLayer.prototype.findPointAt = function(x, y) {
  var bestPoint;
  // This exhaustive search could be optimized by searching nearest tiles
  // first and skipping tiles far away.
  // But since this function is called only when the user clics,
  // speed does not matter. Result correctness is more important.
  for (var key in this.tiles) {
    var candidate = this._closestPointInTile(x, y, key);
    if (candidate && (!bestPoint || bestPoint.dist > candidate.dist)) {
      bestPoint = candidate;
    }
  }
  return bestPoint;
}

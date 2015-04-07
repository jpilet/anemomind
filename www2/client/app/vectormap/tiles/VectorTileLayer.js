function VectorTileLayer(params, renderer) {
  this.params = params;

  this.renderer = renderer;

  this.tiles = {};
  this.numLoading = 0;
  this.loadQueue = [];
  this.numDraw = 0;
  this.numCachedTiles = 0;
  this.visibleCurves = {};

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
  var lastTileX = getTileX(Math.min(this.params.width, bboxBottomRight.x));
  var lastTileY = getTileY(Math.min(this.params.height, bboxBottomRight.y));

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
            this.visibleCurves[curveId] = { };
          }
          this.visibleCurves[curveId][tile.key + c] = curveData;
        }
      }
      return;
    }
  }
};

VectorTileLayer.prototype.drawVisibleCurves = function(context, pinchZoom) {
  if (this.selectedCurve) {
    this.drawCurve(this.selectedCurve, context, pinchZoom);
  } else {
    for (var curveId in this.visibleCurves) {
      this.drawCurve(curveId, context, pinchZoom);
    }
  }
}

VectorTileLayer.prototype.getPointsForCurve = function(curveId) {
  // Extract all points
  var points = [];
  var curveElements = this.visibleCurves[curveId];
  for (var e in curveElements) {
    var element = curveElements[e];
    for (var i in element.points) {
      points.push(element.points[i]);
    }
  }
  // Sort by time
  points.sort(function(a, b) {
    return a.time - b.time;
  });

  this.lastPointArray = points;
  return points;
}

VectorTileLayer.prototype.drawCurve = function(curveId, context, pinchZoom) {
  // prepare the Cavas path

  if (this.isHighlighted(curveId)) {
    context.strokeStyle="#FF0033";
    context.lineWidth = 3;
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

  var points = this.getPointsForCurve(curveId);
  if (points.length == 0) {
    return;
  }

  // Draw.
  context.beginPath();
  var first = pinchZoom.viewerPosFromWorldPos(points[0].pos[0],
                                              points[0].pos[1]);
  context.moveTo(first.x, first.y);
  for (var i = 1; i < points.length; ++i) {
    var point = pinchZoom.viewerPosFromWorldPos(points[i].pos[0],
                                                points[i].pos[1]);
    context.lineTo(point.x, point.y);
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

  var pixelRatio = this.renderer.pixelRatio;

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

  windArrow(getTwdir(p), '#7744ff');

  context.rotate(p.gpsBearing * toRadians);

  var l = 30 * pixelRatio;
  var w = 8 * pixelRatio;
  context.beginPath();
  context.moveTo(0, -l/2);
  context.lineTo(w/2, l/2);
  context.lineTo(-w/2, l/2);
  context.closePath();
  context.fillStyle = '#662200';
  context.fill();

  windArrow(p.awa, '#774400');

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
          timeout: 2000
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
    endTime: new Date(curveId.substr(curveId.length-19)),
    startTime: new Date(curveId.substr(curveId.length-19*2,19))
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
    var loc = this.locationForCurve(curveId);
    if (loc) {
      this.renderer.setLocation(loc);
    }
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
  var endTime= new Date(curveId.substr(curveId.length-19));
  var startTime= new Date(curveId.substr(curveId.length-19*2,19));
  return this.highlight.startTime >= startTime
    && this.highlight.endTime <= endTime;
};

VectorTileLayer.prototype.findPointAt = function(x, y) {
  var p = {x: x, y: y};

  for (var scale = 20; scale >= 0; --scale) {
    var xAtScale = Math.floor(x * (1 << scale));
    var yAtScale = Math.floor(y * (1 << scale));
    var key = scale + "," + xAtScale + "," + yAtScale;

    if (key in this.tiles && this.tiles[key].state == "loaded") {
      var tile = this.tiles[key];
      var bestDist = .25 / (1 << scale);
      var bestPoint = false;
      var bestCurve;

      for (var curve in tile.data) {
        for (var c in tile.data[curve].curves) {
          var curveId = tile.data[curve].curves[c].curveId;
          if (this.selectedCurve && this.selectedCurve != curveId) {
            continue;
          }
          var points = tile.data[curve].curves[c].points;
          for (var i in points) {
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
        return { point: bestPoint, curveId: bestCurve };
      }
      return undefined;
    }
  }
}

VectorTileLayer.prototype.locationForCurve = function(curveId) {
  if (!(curveId in this.visibleCurves)) {
    return undefined;
  }
  var minX = 1000, minY = 1000, maxX = -1000, maxY = -1000;

  // TODO: instead of searching in the visibleCurves array, looking at the tile
  // at scale 0 would be both faster and more accurate, since visibleCurves
  // might only contain partial data.
  var curveElements = this.visibleCurves[curveId];

  for (var e in curveElements) {
    var element = curveElements[e];
    for (var i in element.points) {
      var p = element.points[i].pos;
      minX = Math.min(p[0], minX);
      minY = Math.min(p[1], minY);
      maxX = Math.max(p[0], maxX);
      maxY = Math.max(p[1], maxY);
    }
  }
  return {
    x: (minX + maxX) / 2,
    y: (minY + maxY) / 2,
    scale: 2*Math.max(maxX - minX, maxY - minY)
  };
}

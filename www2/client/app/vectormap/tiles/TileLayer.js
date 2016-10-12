function TileLayer(params, renderer) {
  this.params = params;

  this.renderer = renderer;

  this.tiles = {};
  this.numLoading = 0;
  this.loadQueue = [];
  this.numDraw = 0;
  this.numCachedTiles = 0;

  if (!("tileSize" in this.params)) this.params.tileSize = 256;
  if (!("url" in this.params)) {
    this.params.url = function(scale, x, y) {
      return "http://a.tile.openstreetmap.org/" + scale + '/' + x + '/' + y + '.png';
    };
  }
  if (params.debug) {
      this.debug = function(msg) { console.log(msg); }
  } else {
      this.debug = function(msg) { };
  }
  if (!this.params.maxNumCachedTiles) this.params.maxNumCachedTiles = 64;
  if (!this.params.maxSimultaneousLoads) this.params.maxSimultaneousLoads = 3;

  this.maxUpLevels = this.params.maxUpLevels || 5;

}

TileLayer.prototype.tileSizeOnCanvas = function(canvas) {
  var density = (this.params.forceDevicePixelRatio || window.devicePixelRatio || 1);
  return this.params.tileSize * density;
}

TileLayer.prototype.draw = function(canvas, pinchZoom,
                                    bboxTopLeft, bboxBottomRight) {

  // Compute the scale level
  var numTiles = canvas.width / this.tileSizeOnCanvas();
  var targetUnitPerTile = (bboxBottomRight.x - bboxTopLeft.x) / numTiles;
  var scale = Math.max(0, Math.ceil(- Math.log(targetUnitPerTile) / Math.LN2));

  // Are we in downsampled mode? if yes, artificially push to the next scale level.
  if (canvas.canvasTilesRenderer.params.downsampleDuringMotion
      && pinchZoom.isMoving()) {
    scale += 1;
  }

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

  for (var tileY = firstTileY; tileY <= lastTileY; ++tileY) {
    for (var tileX = firstTileX; tileX <= lastTileX; ++tileX) {
      this.renderTile(scale, tileX, tileY, context, tileGeometry, canvas);
    }
  }

  this.processQueue();

  // control memory usage.
  this.limitCacheSize();

  this.numDraw++;
}

TileLayer.prototype.renderTile = function(scale, tileX, tileY, context, tileGeometry, canvas) {
  var left = tileGeometry.origin.x
      + tileGeometry.delta.x * (tileX - tileGeometry.firstTileX);
  var top = tileGeometry.origin.y
      + tileGeometry.delta.y * (tileY - tileGeometry.firstTileY);

  if (left >= canvas.width || top >= canvas.height) {
    return;
  }

  for (var upLevel = 0; upLevel <= scale && upLevel < this.maxUpLevels; ++upLevel) {
    var upTileX = tileX >> upLevel;
    var upTileY = tileY >> upLevel;
    
    var tile = this.getTile(scale - upLevel, upTileX , upTileY, 1 - upLevel * .15);
    if (tile && tile.image && tile.image.complete && tile.image.width > 0 && tile.image.height > 0) {
      var skipX = tileX - (upTileX << upLevel);
      var skipY = tileY - (upTileY << upLevel);
      var size = this.params.tileSize >> upLevel;
      
      var texCoordX = skipX * size;
      var texCoordY = skipY * size;
      var texWidth = Math.min(size, tile.image.width - skipX * size);
      var texHeight = Math.min(size, tile.image.height - skipY * size);
      
      var width = tileGeometry.delta.x * (texWidth / size);
      var height = tileGeometry.delta.y * (texHeight / size);
      
      try {
          context.drawImage(tile.image,
            texCoordX, texCoordY, texWidth, texHeight,
            left, top, width, height);
      } catch (e) {
          console.log('drawImage failed: ' + e.message);
      }
      return;
    }
  }
};

TileLayer.prototype.tileKey = function(scale, x, y) {
  if (typeof(scale) == 'object') {
    var p = scale;
    scale = p.scale;
    x = p.x;
    y = p.y;
  }
  return  scale + "," + x + "," + y;
}

TileLayer.prototype.getTile = function(scale, x, y, priority) {
  var key = this.tileKey(scale, x, y);
  
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
  
  if (typeof(this.params.url) == "function") {
    var url = this.params.url(scale, x, y);
  } else {
    var url = this.params.url
      .replace("$x", '' + x)
      .replace("$y", '' + y)
      .replace("$scale", '' + scale);
  }
  return this.queueTileRequest(key, url, priority);
};

TileLayer.prototype.queueTileRequest = function(key, url, priority) {
  var tile = { lastDrawRequest: this.numDraw, priority: priority, state: "queue" };
  Utils.assert(tile.priority != undefined);
  this.loadQueue.push({key: key, url: url, tile: tile});
  this.tiles[key] = tile;
  return tile;
};

TileLayer.prototype.processQueue = function() {
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
        t.renderer.loadImage(query.url,
          function(image) {
            t.numLoading--;
            t.numCachedTiles++;
            query.tile.state = "loaded";
            query.tile.image = image;
            t.renderer.refreshIfNotMoving();
          },
          function(error) {
            console.log('Failed to load image: ' + query.url + ': ' + error);
            t.numLoading--;
            query.tile.state = "failed";
            t.processQueue();
          });
      })(this, query);


    } else {
      // There's no need to load this tile, it is not required anymore.
      delete this.tiles[query.key];
    }
  }
};

TileLayer.prototype.limitCacheSize = function() {
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

TileLayer.prototype.maxZoomAt = function(p) {
  var maxScale = undefined;
  var firstFailed

  var me = this;
  var stateAtScale = function(scale) {
    var key = me.tileKey(Utils.worldToTile(scale, p));
    if (key in me.tiles) {
      return me.tiles[key].state;
    }
    return undefined;
  };

  var lastTileWithState = function(state) {
    for (var i = 20; i >= 0; --i) {
      if (stateAtScale(i) == state) {
        return i;
      }
    }
    return undefined;
  };

  var lastAvailable = lastTileWithState('loaded');
  if (lastAvailable != undefined) {
    if (stateAtScale(lastAvailable + 1) == 'failed') {
      return lastAvailable;
    }
  }

  return undefined;
};

TileLayer.prototype.minScaleAt = function(canvas, p) {
  var maxZoomLevel = this.maxZoomAt(p);
  if (maxZoomLevel == undefined) {
    return undefined;
  }

  var numTiles = canvas.width / this.tileSizeOnCanvas(canvas);
  var minScale = .5 * numTiles / (1 << maxZoomLevel);

  return minScale;
};


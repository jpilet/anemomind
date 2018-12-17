function DynLoader(fetchTile) {
  this.maxSimultaneousLoads = 2;
  this.maxTilesInCache = 256;
  this.setFetchTile(fetchTile);

  // These values should match those in CharTiles.h
  this.minZoom = 9;
  this.maxZoom = 28;
  this.samplesPerTile = 512;

  this.pixelsPerSample = 4;
}

DynLoader.prototype.states = {
  LOADED: "loaded",
  LOADING: "loading",
  QUEUED: "queued",
  FAILED: "failed",
  WONTFETCH: "wontfetch"
};

DynLoader.prototype.setFetchTile = function(fetchTile) {
  this.tiles = {};
  this.fetchTile = fetchTile;
  this.viewCount = 0;
};

DynLoader.prototype.removeAllQueued = function() {
  var QUEUED = this.states.QUEUED;
  for (var key in this.tiles) {
    var tile = this.tiles[key];
    if (tile.state == QUEUED) {
      delete this.tiles[key];
    }
  }
};

DynLoader.prototype.enqueueTile = function(tile) {
  var key = this.tileKey(tile);
  var state = this.stateOfTile(key);
  if (state == this.states.WONTFETCH) {
    this.tiles[key] = {
      z: tile.z,
      t: tile.t,
      state: this.states.QUEUED
    };
  }
  return key;
};

DynLoader.prototype.processQueue = function() {
  var me = this;
  var countState = function(state) {
    var count = 0;
    for (var i in me.tiles) {
      if (me.tiles[i].state == state) {
        ++count;
      }
    }
    return count;
  };

  var numLoading = countState(this.states.LOADING);
  var toExec = this.maxSimultaneousLoads - numLoading;
  for (var key in this.tiles) {
    if (toExec <= 0) {
      break;
    }

    var tile = this.tiles[key];
    if (tile.state == this.states.QUEUED) {
      tile.state = this.states.LOADING;
      this.fetchTile(tile.z, tile.t);
      --toExec;
    }
  }
};

DynLoader.prototype.tileRangeInView = function(scale) {
  var range = scale.range();
  var pixels = range[1] - range[0];
  var startTime = scale.invert(range[0]);
  var endTime = scale.invert(range[1]);

  var timeRangeSec = (endTime - startTime) / 1000;
  var samplesPerTile = this.samplesPerTile;
  var pixelsPerSample = this.pixelsPerSample;
  var wantedTimePerTile = timeRangeSec * samplesPerTile * pixelsPerSample / pixels;
  var zoom = Math.floor(Math.log2(wantedTimePerTile));
  zoom = Math.min(this.maxZoom, Math.max(this.minZoom, zoom));

  var actualTimePerTile = 1 << zoom;

  var tileAtTime = function(time) {
    return Math.floor(time.getTime() / 1000 / actualTimePerTile);
  };

  return {
    firstTile: tileAtTime(startTime),
    lastTile: tileAtTime(endTime),
    zoom: zoom
  };
};

DynLoader.prototype.tilesInView = function(scale) {
  var range = this.tileRangeInView(scale);

  var tilesInView = [];

  for (var i = range.firstTile; i <= range.lastTile; ++i) {
    tilesInView.push({ z: range.zoom, t: i });
  }
  return tilesInView;
};

DynLoader.prototype.getTile = function(tile) {
  var key = this.tileKey(tile);
  if (key in this.tiles) { 
    return this.tiles[key];
  }
  return undefined;
};

DynLoader.prototype.tileKey = function(tile) {
  if (typeof(tile) == 'string') {
    // already a key?
    return tile;
  } else if ('z' in tile) {
    return tile.z + ',' + tile.t;
  }
  throw new Error("tileKey expects a tile");
};

DynLoader.prototype.stateOfTile = function(tile) {
  var k = this.tileKey(tile);
  if (k in this.tiles) {
    return this.tiles[k].state;
  }
  return this.states.WONTFETCH;
};

DynLoader.prototype.fetchTilesInView = function(range) {
  var me = this;
  var newFetchCount = 0;
  this.removeAllQueued();
  this.viewCount += 1;
  this.tilesInView(range).forEach(function(t) {
    var key = me.enqueueTile(t);
    me.tiles[key].age = me.viewCount;
  });
  this.processQueue();
  this.deleteOldTiles();
};

DynLoader.prototype.getOrMakeTile = function (z, t) {
  var zt = {z: z, t: t};
  var key = this.tileKey(zt);
  if (!(key in this.tiles)) {
    return this.tiles[key] = zt;
  }
  return this.tiles[key];
}

DynLoader.prototype.tileFetchSucceeded = function (z, t, data) {
  var tile = this.getOrMakeTile(z, t);
  tile.data = data;
  tile.state = this.states.LOADED;
};

DynLoader.prototype.tileFetchFailed = function (z, t) {
  var tile = this.getOrMakeTile(z, t);
  tile.state = this.states.FAILED;
};

DynLoader.prototype.deleteOldTiles = function() {
  if (Object.keys) { // ES5 
    var length = Object.keys(this.tiles).length;
    if (length < this.maxTilesInCache) {
      return;
    }
  }
  var tileArray = [];
  for (var key in this.tiles) {
    var tile = this.tiles[key];
    if ('age' in tile && tile.state == this.states.LOADED) {
      tileArray.push({key: key, age: tile.age});
    }
  }
  var numToRemove = tileArray.length - this.maxTilesInCache;
  
  if (numToRemove <= 0) {
    return;
  }
  
  // sort by age, oldest first
  tileArray.sort(function(a, b) { return a.age - b.age; });
  
  for (var i = 0; i < numToRemove; ++i) {
    delete this.tiles[tileArray[i].key];
  }
};

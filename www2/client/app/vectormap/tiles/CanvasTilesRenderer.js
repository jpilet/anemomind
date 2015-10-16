// CanvasTilesRenderer.js - copyright Julien Pilet, 2013

/** Construct a CanvasTilesRenderer object on a canvas.
 *
 * \param canvas the canvas element to render tiles to.
 * \param url a function taking (scale,x,y) as arguments. Returns a tile URL.
 *            default: openstreetmap.
 * \param initialLocation the initial location (zoom and translation).
 *                        See setLocation().
 * \param tileSize the tileSize, in pixels. Default: 256.
 * \param width the image width. Unit: number of tiles at level 0 (float OK).
 * \param height image height. Unit: number of tiles at level 0.
 * \param maxNumCachedTiles the maximum number of hidden tiles to keep in cache.
 *                          default: 64.
 * \param maxSimultaneousLoads maximum parallel loading operations. Default: 3.
 * \param downgradeIfSlowerFPS If rendering falls below this framerate, degrade
 *                             image quality during animation.
 * \param debug if true, output debug info on console.
 */
function CanvasTilesRenderer(params) {
  this.params = (params != undefined ? params : {});
  this.canvas = params.canvas;
  this.canvas.canvasTilesRenderer = this;
  
  this.params.width = this.params.width || 1;
  this.params.height = this.params.height || 1;
  this.params.minScale = this.params.minScale || 0;
  
  this.params.downgradeIfSlowerFPS = params.downgradeIfSlowerFPS || 20;
  
  this.layers = [
    new TileLayer(params, this),
    new VectorTileLayer(params, this)
  ];

  this.canvasWidth = -1;
  this.canvasHeight = -1;

  if (params.debug) {
      this.debug = function(msg) { console.log(msg); }
  } else {
      this.debug = function(msg) { };
  }

  // Block drawing before we are ready.  
  this.inDraw = true;
  this.numDraw = 0;
  this.disableResize = false;
  this.lastRefreshRequest = -1;
  
  var t = this;
  this.pinchZoom = new PinchZoom(t.canvas, function() {
    t.location = t.getLocation();
    if (t.params.onLocationChange) { t.params.onLocationChange(t); }
    if (t.params.debug) {
      t.debug('location: w:' + t.canvas.width
              + ' h:' + t.canvas.height
              + ' x:'+t.location.x + ' y:'+t.location.y
              +' s:'+t.location.scale);
    }
    t.refresh();
  },
  this.params.width,
  this.params.height);
  this.pinchZoom.minScale = this.params.minScale;

  // We are ready, let's allow drawing.  
  this.inDraw = false;
  
  this.pinchZoom.onClic = function(pos) { };

  var location = params.initialLocation || {
    x: (this.params.width || 1) / 2,
    y: (this.params.height || 1) / 2,
    scale: (this.params.width || 1)
  };
  this.setLocation(location);
}

/** Get the current view location.
 *
 *  Returns an object containing:
 *  x: the x coordinate currently in the center of the screen, in tile 0 size units.
 *  y: the corresponding y coordinate.
 *  scale: the viewport width, in "tile 0" units.
 */
CanvasTilesRenderer.prototype.getLocation = function() {
  var left = this.pinchZoom.worldPosFromViewerPos({x: 0, y:this.canvas.height / 2});
  var right = this.pinchZoom.worldPosFromViewerPos({x: this.canvas.width, y:this.canvas.height / 2});
  
  return {
    x: (left.x + right.x) / 2,
    y: (left.y + right.y) / 2,
    scale: Utils.distance(right, left)
  };
};

/** Set the current location.
 *
 * \param location the location in the format returned by getLocation().
 */
CanvasTilesRenderer.prototype.setLocation = function(location) {
  if (isNaN(location.x) || isNaN(location.y) || isNaN(location.scale)) {
    throw('invalid location');
  }
  var canvas = this.canvas;
  var ratio = [
    location['vx'] || .5,
    location['vy'] || .5,
  ];
  var x_pos = canvas.width * ratio[0];
  var y_pos = canvas.height * ratio[1];
  var constraints = [
    { viewer: {x: x_pos - canvas.width / 2, y: y_pos}, world: {x:location.x - location.scale /2, y: location.y} },
    { viewer: {x: x_pos + canvas.width / 2, y: y_pos}, world: {x:location.x + location.scale /2, y: location.y} },
  ];
  this.location = location;
  this.pinchZoom.processConstraints(constraints);  
};

/** Refresh the canvas.
 *
 * this method has to be called if the canvas element is resized.
 * calling refresh mutliple times in a raw only causes 1 refresh to occur.
 */
CanvasTilesRenderer.prototype.refresh = function() {
  var t = this;
  if (t.lastRefreshRequest == t.numDraw) {
    return;
  }
  t.lastRefreshRequest = t.numDraw;

  window.requestAnimationFrame(function() { t.draw(t.canvas); });
};

CanvasTilesRenderer.prototype.resizeCanvas = function(width, height) {
  if (this.disableResize) {
    return;
  }

  var canvas = this.canvas;

  // devicePixelRatio should tell us about the current zoom level.
  var density = (this.params.forceDevicePixelRatio || window.devicePixelRatio || 1);

  // On some browsers/devices, filling the full resolution canvas
  // takes too long. During animations, we downsample the canvas
  // to make it fast enough. When motion stops, we render the map
  // at full resolution again.
  var factor = (this.params.downsampleDuringMotion &&
                this.pinchZoom.isMoving()) ?  density / 2 : density;

  var initialClientWidth = canvas.clientWidth;
  var initialClientHeight = canvas.clientHeight;

  // Is it Math.floor or Math.round ? Who knows...
  var newWidth = width || Math.floor(canvas.clientWidth * factor);
  var newHeight = height || Math.floor(canvas.clientHeight * factor);

  if (newWidth != 0 && newHeight != 0 && 
         (Math.abs(canvas.width - newWidth) > 3 ||
          Math.abs(canvas.height - newHeight) > 3)) {
      canvas.width = newWidth;
      canvas.height = newHeight;
      this.pixelRatio = factor;
  }

  if (canvas.width != this.canvasWidth || canvas.height != this.canvasHeight) {
      this.canvasWidth = canvas.width;
      this.canvasHeight = canvas.height;

      // We resized the canvas, but we want to express the same transform.
      // Let's update the transform for the new size.
      this.setLocation(this.location);
  }  
  
  if (width == undefined && height == undefined
      && (initialClientWidth != canvas.clientWidth
          || initialClientHeight != canvas.clientHeight)) {
     // Canvas size on page should be set by CSS, not by canvas.width and canvas.height.
     // It seems it is not the case. Let's forget about this devicePixelRatio stuff.
     this.disableResize = true;  
     this.debug('Disabling resize :(');
  }  
};

CanvasTilesRenderer.prototype.draw = function() {
  if (this.inDraw) {
    return;
  }
  this.inDraw = true;

  var startTimestamp = new Date().getTime();

  var canvas = this.canvas;
  var pinchZoom = this.pinchZoom;
  
  this.resizeCanvas();
  pinchZoom.checkAndApplyTransform();
  
  // Compute a bounding box of the viewer area in world coordinates.
  var cornersPix = [
    {x: 0, y: 0},
    {x: canvas.width, y: 0},
    {x: canvas.width, y: canvas.height},
    {x: 0, y: canvas.height},
  ];
  var cornersWorld = [];
  for (var i = 0; i < 4; ++i) {
    cornersWorld.push(pinchZoom.worldPosFromViewerPos(cornersPix[i]));
  }
  var bboxTopLeft = cornersWorld.reduce(
    function(a,b) { return {x: Math.min(a.x, b.x), y: Math.min(a.y, b.y)}; },
    cornersWorld[0]);
    
  var bboxBottomRight = cornersWorld.reduce(
    function(a,b) { return {x: Math.max(a.x, b.x), y: Math.max(a.y, b.y)}; },
    cornersWorld[0]);
    
  // Clear the canvas
  var context = canvas.getContext('2d');

  this.clearBorder(context);

  for (var i in this.layers) {
    this.layers[i].draw(canvas, pinchZoom, bboxTopLeft, bboxBottomRight);
  }

  // Rendering resolution is decreased during motion.
  // To render high-res after a motion, we detect motion end
  // by setting and postponing a timeout during motion.
  var moving = this.pinchZoom.isMoving();
  if (moving) {
    if (this.moveEndTimeout != undefined) {
      window.clearTimeout(this.moveEndTimeout);
    }
    var t = this;
    this.moveEndTimeout = setTimeout(function() {
      t.moveEndTimeout = undefined;
      t.refresh();
    }, 100);
  }
  
  this.inDraw = false;
  ++this.numDraw;

  var endTimestamp = new Date().getTime();
  var renderingTime = (endTimestamp - startTimestamp);
  if (renderingTime > (1000/this.params.downgradeIfSlowerFPS)) {
      // If rendering is too slow, we degrade visual quality during motion,
      // to make it faster.
      this.params.downsampleDuringMotion = true;
  }

  if (this.params.debug) {
    var debugLocation = this.getLocation();
    this.debug('Draw at '
               + debugLocation.x + ', ' + debugLocation.y +' scale: ' + debugLocation.scale
               + ' rendering time:' + renderingTime
               + ' w:' + canvas.width + ' h:' + canvas.height);
  }
};


CanvasTilesRenderer.prototype.clearBorder = function(context) {
  var canvas = this.canvas;

  var topLeft = this.pinchZoom.viewerPosFromWorldPos(0, 0);
  var bottomRight = this.pinchZoom.viewerPosFromWorldPos(this.params.width,
                                               this.params.height);

  context.fillStyle = 'white';
  if (topLeft.x > 0) {
    context.fillRect(0, 0, Math.floor(topLeft.x), canvas.height);
  }

  if (topLeft.y > 0) {
    context.fillRect(0, 0, canvas.width, Math.floor(topLeft.y));
  }

  if (bottomRight.x < canvas.width) {
    context.fillRect(bottomRight.x, 0,
                      canvas.width - bottomRight.x, canvas.height);
  }
  if (bottomRight.y < canvas.height) {
    context.fillRect(0, bottomRight.y,
                      canvas.width, canvas.height - bottomRight.y);
  }
};

CanvasTilesRenderer.prototype.refreshIfNotMoving = function() {
  if (!this.pinchZoom.isMoving()) {
    this.refresh();
  }
}

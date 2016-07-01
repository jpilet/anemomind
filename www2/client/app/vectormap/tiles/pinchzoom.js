/**
 * @author Julien Pilet
 */

function PinchZoom(element, transformChanged, width, height, disabledZoom) {
  this.ongoingTouches = {};
  this.transform = new AffineTransform();
  this.transformChanged = transformChanged;
  this.worldWidth = width || 1;
  this.worldHeight = height || 1;
  this.lastMouseDown = 0;
  this.lastMouseUp = 0;
  this.lastTouchDown = 0;
  this.lastTouchDownPos = {x:-1, y:-1};
  this.minScale = 0;
  
  var t = this;
  var e = element;
  if(!disabledZoom){
    e.addEventListener("touchstart", function(event) { t.handleStart(event); }, false);
    e.addEventListener("touchend", function(event) { t.handleEnd(event); }, false);
    e.addEventListener("touchcancel", function(event) { t.handleEnd(event); }, false);
    e.addEventListener("touchmove", function(event) { t.handleMove(event); }, false);
    e.addEventListener("mousedown", function(event) { t.handleMouseDown(event); }, false);
    e.addEventListener("mousemove", function(event) { t.handleMouseMove(event); }, false);
    e.addEventListener("mouseup", function(event) { t.handleMouseUp(event); }, false);
    addWheelListener(element, function(event) { t.handleMouseWheel(event); });
  }  
  element.pinchZoomInstance = this;
  this.element = element;
}


PinchZoom.prototype.eventElement = function(event) {
  if (event.srcElement) { return event.srcElement; }
  else if (event.currentTarget) { return event.currentTarget; }
  else {
    return undefined;
  }
};

PinchZoom.prototype.setTransform = function(transform) {
  var newTransform = new AffineTransform(transform);
  if (!this.checkAndApplyTransform(newTransform)) {
    return;
  }

  // The transform has changed: ongoing gesture has to be reset.
  var viewerPos = Utils.eventPosInElementCoordinates(event, this.eventElement(event));
  if (this.ongoingTouches.mouse) {
    var viewerPos =  this.ongoingTouches.mouse.startViewerPos;
    this.ongoingTouches.mouse.startWorldPos = this.worldPosFromViewerPos(viewerPos.x, viewerPos.y);
  } else {
    this.ongoingTouches = {};
  }
};

PinchZoom.prototype.worldPosFromViewerPos = function(x, y) {
  return this.transform.inverseTransform(x, y);
};

PinchZoom.prototype.viewerPosFromWorldPos = function(x, y) {
  return this.transform.transform(x, y);
};

PinchZoom.prototype.isMoving = function() {
    var count = 0;

    for (var i in this.ongoingTouches) {
        if (this.ongoingTouches[i]) {
            ++count;
        }
    }
    return count > 0;
};

PinchZoom.prototype.handleDoubleClic = function(viewerPos) {
    var constraints = [{
      viewer: viewerPos,
      world: this.worldPosFromViewerPos(viewerPos.x, viewerPos.y),
    }];
  
    this.transform.scale(2);
    this.processConstraints(constraints);
};

PinchZoom.prototype.clicPosFromViewerPos = function(viewerPos) {
  return {
      startWorldPos: this.worldPosFromViewerPos(viewerPos.x, viewerPos.y),
      startViewerPos: viewerPos,
    };
};

PinchZoom.prototype.handleMouseDown = function(event) {
  var viewerPos = Utils.eventPosInElementCoordinates(event, this.element);

  var now = event.timeStamp;
  if ((now - this.lastMouseDown) < 200) {
    // double clic: instant zoom.
    this.handleDoubleClic(viewerPos);
    this.ongoingTouches.mouse = undefined;
    clearTimeout(this.singleClicTimeout);
  } else {
    // simple clic - might be converted in a double clic later.
    var clicPos = this.clicPosFromViewerPos(viewerPos);
    this.ongoingTouches.mouse = clicPos;
    var t = this;
    this.singleClicTimeout = setTimeout(
        function() {
          // Make sure this is not a long clic or a drag.
          if (t.lastMouseUp > t.lastMouseDown &&
              (t.lastMouseUp - t.lastMouseDown) < 200) {
            t.handleSingleClic(clicPos);
          }
        }, 200);
  }
  this.lastMouseDown = now;
};

PinchZoom.prototype.handleMouseUp = function(event) {
  this.lastMouseUp = event.timeStamp;
  event.preventDefault();
  this.handleMouseMove(event);
  this.ongoingTouches.mouse = undefined;
};

PinchZoom.prototype.handleMouseMove = function(event) {
  event.preventDefault();
  
  if (this.ongoingTouches.mouse) {
    this.ongoingTouches.mouse.currentViewerPos = Utils.eventPosInElementCoordinates(event, this.element);
    var constraints = [{
      viewer: this.ongoingTouches.mouse.currentViewerPos,
      world: this.ongoingTouches.mouse.startWorldPos,
    }];
    this.processConstraints(constraints);
  }
};

PinchZoom.prototype.handleMouseWheel = function(event) {
  event.preventDefault();
  
  var viewerPos = Utils.eventPosInElementCoordinates(event, this.element);
  var constraints = [{
      viewer: viewerPos,
      world: this.worldPosFromViewerPos(viewerPos.x, viewerPos.y),
  }];
  var scaleFactor = 1.0 - Math.max(-.2, Math.min(.2, event.deltaY / 20.0));
  
  this.transform.scale(scaleFactor);
  this.processConstraints(constraints);
};

PinchZoom.prototype.handleStart = function(event) {
  event.preventDefault();

  // Detect double clic, single touch.
  if (event.touches.length == 1) {
    var now = event.timeStamp;
    var delta = (now - this.lastTouchDown);
    var viewerPos = Utils.eventPosInElementCoordinates(
        event.touches[0], this.element);
    var dist = Utils.distance(viewerPos, this.lastTouchDownPos);
    // double tap ?
    if (delta < 300 && dist < 100) {
      this.handleDoubleClic(viewerPos);
    }
    this.lastTouchDown = now;
    this.lastTouchDownPos = viewerPos;
  }

  var touches = event.changedTouches;
  for (var i = 0; i < touches.length; i++) {
    var viewerPos = Utils.eventPosInElementCoordinates(touches[i], this.element);

    this.ongoingTouches[touches[i].identifier] = {
startWorldPos: this.worldPosFromViewerPos(viewerPos.x, viewerPos.y),
               startViewerPos: viewerPos,
    };
  }
};
	
PinchZoom.prototype.handleEnd = function(event) {
  event.preventDefault();

  if (event.touches.length == 0) {
    // No finger left on the screen. Was it a single tap?
    var now = event.timeStamp;
    var delta = (now - this.lastTouchDown);
    if (delta < 300) {
      // yes!
      for (var i in this.ongoingTouches) {
        this.handleSingleClic(this.ongoingTouches[i]);
        break;
      }
      this.ongoingTouches = {};
      return;
    }
  }

  // If one finger leaves the screen, we forget all finger positions. Thus, it
  // starts a new motion if some other fingers keep moving.
  this.ongoingTouches = {};
  this.handleMove(event);
};

PinchZoom.prototype.handleMove = function(event) {
  event.preventDefault();
  var touches = event.touches;
  var constraints = [];
  for (var i = 0; i < touches.length; i++) {
		if (!this.ongoingTouches[touches[i].identifier]) {
			// For some reason, we did not get the start event.
			var viewerPos = Utils.eventPosInElementCoordinates(touches[i], this.element);
		  this.ongoingTouches[touches[i].identifier] = {
			  startWorldPos: this.worldPosFromViewerPos(viewerPos.x, viewerPos.y),
			  startViewerPos: viewerPos,
		  };
		}
		var touch = this.ongoingTouches[touches[i].identifier];
		
		// Every touch is a constraint
		constraints.push({
			viewer: Utils.eventPosInElementCoordinates(touches[i], this.element),
			world: touch.startWorldPos,
		});
  }
  this.processConstraints(constraints);
};

PinchZoom.prototype.processConstraints = function(constraints) {
  // Compute the transform that best fits the constraints
  var newTransform = new AffineTransform(this.transform.matrix);
  var T = newTransform.matrix;

  if (constraints.length >= 2) {
    // pinch -> zoom
    // solve:
    //   s * worldx + tx = viewerx
    //   s * worldy + ty = viewery
    // For each constraint:
    //  worldx 1 0  * s  = viewerx
    //  worldy 0 1    tx   viewery
    //                ty
    // Let A be the 4 by 3 matrix composed of the two constraints, as shown above.
    // The solution is computed with: [s tx ty]' = inverse(A' * A) * A' [viewerx viewery]'
    var wx1 = constraints[0].world.x;
    var wy1 = constraints[0].world.y;
    var wx2 = constraints[1].world.x;
    var wy2 = constraints[1].world.y;
    var vx1 = constraints[0].viewer.x;
    var vy1 = constraints[0].viewer.y;
    var vx2 = constraints[1].viewer.x;
    var vy2 = constraints[1].viewer.y;

    var AtA00 = wx1*wx1 + wx2*wx2 + wy1*wy1 + wy2*wy2;
    var AtA10 = wx1 + wx2;
    var AtA20 = wy1 + wy2;
    var Ainv = Utils.invert3x3Matrix([AtA00, AtA10, AtA20, AtA10, 2, 0, AtA20, 0, 2]);
    var AtB = [vx1*wx1 + vx2*wx2 + vy1*wy1 + vy2*wy2, vx1 + vx2, vy1 + vy2];
    var r = Utils.multiply3x3MatrixWithVector(Ainv, AtB);

    T[0] = T[4] = r[0];
    T[2] = r[1];
    T[5] = r[2];    

    this.enforceConstraints(newTransform);

    // If enforceConstraints changed scale, we need to reset translation.
    var c = {
      world: {
        x: (constraints[0].world.x + constraints[1].world.x) / 2,
        y: (constraints[0].world.y + constraints[1].world.y) / 2,
      },
      viewer: {
        x: (constraints[0].viewer.x + constraints[1].viewer.x) / 2,
        y: (constraints[0].viewer.y + constraints[1].viewer.y) / 2,
      }
    };
    T[2] = c.viewer.x - (T[0] * c.world.x + T[1] * c.world.y);
    T[5] = c.viewer.y - (T[3] * c.world.x + T[4] * c.world.y);

  } else if (constraints.length == 1) {
    // Make sure the scale is within bounds.
    this.enforceConstraints(newTransform);

    // scroll: Solve A* world + X = viewer
    // -> X = viewer - A * world
    var c = constraints[0];
    T[2] = c.viewer.x - (T[0] * c.world.x + T[1] * c.world.y);
    T[5] = c.viewer.y - (T[3] * c.world.x + T[4] * c.world.y);
  }

  var tiledViewer = this;
  
  this.checkAndApplyTransform(newTransform);
};

PinchZoom.prototype.enforceConstraints = function (newTransform) {
  var T = newTransform.matrix;

  var boundScaleX = this.element.width / this.worldWidth;
  var boundScaleY = this.element.height / this.worldHeight;
  var scaleBound = Math.min(boundScaleX, boundScaleY);

  var scale = T[0];
  var scaleFactor = 1.0;
  if (scale < scaleBound) {
    scaleFactor = scaleBound / scale;
  }

  if (this.minScale > 0) {
      var maxScale = this.element.width / this.minScale;
      if (scale > maxScale) {
          scaleFactor = maxScale / scale;
      }
  }

  T[0] = T[4] = scale * scaleFactor;
  T[2] *= scaleFactor;
  T[5] *= scaleFactor;
    
  if (T[2] > 0) T[2] = 0;
  if (T[5] > 0) T[5] = 0;    
  
  var bottomright = newTransform.transform(this.worldWidth, this.worldHeight);
  if (bottomright.x < this.element.width) {
    var center = (T[2] == 0 ? .5 : 1);
    T[2] += center * (this.element.width - bottomright.x);      
  }
  if (bottomright.y < this.element.height) {
    var center = (T[5] == 0 ? .5 : 1);
    T[5] += center * (this.element.height - bottomright.y);      
  }
};
  
PinchZoom.prototype.checkAndApplyTransform = function (newTransform) {
  newTransform = newTransform || this.transform;

  this.enforceConstraints(newTransform);

  if (this.transform !== newTransform) {
    this.transform = newTransform;
    if (this.transformChanged) {
      this.transformChanged(this.transform);
    }
  }
};

PinchZoom.prototype.handleSingleClic = function(mousePos) {
  if (this.onClic) {
    this.onClic(mousePos, this);
  }
}

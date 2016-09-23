function ScaleLayer(params, renderer) {
  this.renderer = renderer;
  this.params = params || {};
  this.sizeRatio = this.params.sizeRatio || .3;
  this.margin = this.params.margin || 15;
  if (typeof(this.margin) == 'number') {
    this.margin = [this.margin, this.margin];
  }

  this.verticalPlacement = params.verticalPlacement || 'top';
  this.horizontalPlacement = params.horizontalPlacement || 'right';
  this.shadowColor = params.shadowColor || 'rgba(255,255,255,.8)';
  this.scaleColor = params.scaleColor || '#000000';
}

function nice(x) {
  var r = Math.pow(10, Math.floor(Math.log10(x))) / 2;
  return Math.round(x / r) * r;
}

function formatScale(km) {
  if (km <= 1) {
    return '~ ' + (km * 1000).toFixed(0) + ' m';
  } else {
    return '~ ' + km.toFixed(0) + ' km';
  }
}

ScaleLayer.prototype.draw = function(canvas, pinchZoom,
                                     bboxTopLeft, bboxBottomRight) {
  var pixelRatio = this.renderer.pixelRatio;
  if ((this.params.minCanvasWidth * pixelRatio) > canvas.width) {
    // screen too small.
    return;
  }

  var latLon = Utils.worldToLatLon(
      pinchZoom.worldPosFromViewerPos({x: canvas.width/2, y: canvas.height/2}));

  var earthCircumference = 40075.017; // in km
  var xToKm = earthCircumference * Math.cos(latLon.lat * Math.PI / 180);
  var scaleWorld =
    Math.min(pinchZoom.bottomRightWorld().x, bboxBottomRight.x)
    - Math.max(pinchZoom.topLeftWorld().x, bboxTopLeft.x);

  if (scaleWorld <= 0) {
    // this should never happen.
    return;
  }

  var scaleKm = nice(scaleWorld / 4 * xToKm);

  if (this.params.maxDist && scaleKm > this.params.maxDist) {
    // for very large distances, it does not really make sense
    // to have a linear scale: the mercator projection distorts too much.
    return;
  }


  var marginX = this.margin[0] * pixelRatio;
  var marginY = this.margin[1] * pixelRatio;

  var topLeftViewer =
    pinchZoom.viewerPosFromWorldPos(pinchZoom.topLeftWorld());
  var bottomRightViewer =
    pinchZoom.viewerPosFromWorldPos(pinchZoom.bottomRightWorld());

  var startViewer = {
    x: (this.horizontalPlacement == 'right' ?
        Math.min(canvas.width, bottomRightViewer.x) - marginX
        : Math.max(0, topLeftViewer.x) + marginX),
    y: (this.verticalPlacement == 'top' ?
        Math.max(topLeftViewer.y, 0) + marginY
        : Math.min(bottomRightViewer.y, canvas.height) - marginY)
  };

  var startWorld = pinchZoom.worldPosFromViewerPos(startViewer);
  var endWorld = {
    x: startWorld.x + scaleKm / xToKm * (this.horizontalPlacement == 'right' ? -1 : 1),
    y: startWorld.y
  };
  var endViewer = pinchZoom.viewerPosFromWorldPos(endWorld);
  endViewer.x = Math.round(endViewer.x);
  endViewer.y = Math.round(endViewer.y);

  if (endViewer.x > canvas.width || endViewer.x <= marginX) {
    // oops.. the screen is too thin.
    return;
  }

  var context = canvas.getContext('2d');

  var dy = 4 * pixelRatio;

  for (var pass = 0; pass < 2; ++pass) {
    context.strokeStyle = (pass == 0 ? this.shadowColor : this.scaleColor);
    context.lineWidth = (pass == 0 ? 3 : 1) * pixelRatio;

    context.beginPath();
    context.moveTo(startViewer.x, startViewer.y);
    context.lineTo(endViewer.x, endViewer.y);

    context.moveTo(startViewer.x, startViewer.y - dy);
    context.lineTo(startViewer.x, startViewer.y + dy);

    context.moveTo(endViewer.x, endViewer.y - dy);
    context.lineTo(endViewer.x, endViewer.y + dy);

    context.stroke();
  }

  renderShadowedText(context, pixelRatio, formatScale(scaleKm),
    startViewer.x + dy * (this.horizontalPlacement == 'right' ? -1 : 1),
    startViewer.y,
    {
      endOrStart: (this.horizontalPlacement == 'right' ? 'end' : 'start'),
      topOrBottom: (this.verticalPlacement == 'top' ? 'top' : 'bottom'),
      color: this.scaleColor,
      shadowColor: this.shadowColor
    }
  );
};

/* Options:
     endOrStart,
     topOrBottom,
     color, shadowColor,
     fontSize,
     font
     measureCondition
*/
function renderShadowedText(context, pixelRatio, text, x, y, options) {

  context.textAlign = (options.endOrStart == 'end' ? 'end' : 'start');
  context.textBaseline = (options.topOrBottom == 'top' ? 'top' : 'bottom');

  var fontSize = (options.fontSize || 12) * pixelRatio;
  var font = options.font || 
      'Roboto, "Helvetica Neue", HelveticaNeue, "Helvetica-Neue", Helvetica, Arial, "Lucida Grande", sans-serif';
  context.font = fontSize + 'px ' + font;

  context.strokeStyle = options.shadowColor || '#fff';
  context.lineWidth = 3 * pixelRatio;
  context.fillStyle = options.color || '#000';

  if (options.measureCondition
      && !options.measureCondition(context.measureText(text))) {
    return;
  }

  context.strokeText(text, x, y);
  context.fillText(text, x, y);
}

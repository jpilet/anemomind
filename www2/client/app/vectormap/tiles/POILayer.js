function POILayer(params) {
  this.params = params;
  this.renderer = params.renderer;
  if (!this.renderer) {
    throw(new Error("POILayer: no renderer !"));
  }
  
  this.icons = { };

  params.radius = params.radius || 16;
  params.color = params.color || '#008800';

  if (params.debug) {
      this.debug = function(msg) { console.log(msg); }
  } else {
      this.debug = function(msg) { };
  }

  this.renderer.addLayer(this);
  var me = this;

  if (this.params.onFeatureClic) {
    this.renderer.addClicHandler(function(pos) {
      return me.handleClic(pos);
    });
  }
}

function forEachFeature(geojson, callback) {
  if (geojson.type == "FeatureCollection") {
    var n = geojson.features.length;
    for (var i = 0; i < n; ++i) {
      forEachFeature(geojson.features[i], callback);
    }
  } else if (geojson.type == "Feature") {
    callback(geojson);
  }
}

function geojsonGetCoordinates(feature) {
  var geom = feature.geometry;
  if (!geom) {
    return undefined;
  }

  if (!geom.osmCoordinates) {
    var coord = Utils.latLonToWorld(geom.coordinates);
    geom.osmCoordinates = coord;
  } else {
    var coord = geom.osmCoordinates;
  }

  return coord;
}

POILayer.prototype.draw = function(canvas, pinchZoom,
                                   bboxTopLeft, bboxBottomRight) {
  var geojson = this.params.geojson;

  var context = canvas.getContext('2d');
  context.strokeStyle = '#003300';
  context.lineWidth = 1;

  this.delayedFeatures = [];
  this.renderingTop = false;

  var me = this;
  forEachFeature(geojson, function(feature) {
    me.renderFeature(canvas, pinchZoom, feature, context);
  });

  this.renderingTop = true;
  for (var i in this.delayedFeatures) {
    this.renderFeature(canvas, pinchZoom, this.delayedFeatures[i], context);
  }
}

POILayer.prototype.renderFeature = function(canvas, pinchZoom, geojson, context) {
  var type = geojson.geometry && geojson.geometry.type;
  if (!type) {
    return;
  }

  var renderOnTop = geojson.properties && geojson.properties.renderOnTop;
  var renderingTop = this.renderingTop;
  if (renderOnTop && !renderingTop) {
    this.delayedFeatures.push(geojson);
    return;
  }
  if (!renderOnTop && renderingTop) {
    return;
  }

  var funcName = 'render' + type;
  if (this[funcName]) {
    this[funcName](canvas, pinchZoom, geojson, context);
  }
}

POILayer.prototype.featureRadius = function() {
  return this.params.radius * this.renderer.pixelRatio;
}

POILayer.prototype.renderPoint = function(canvas, pinchZoom, geojson, context) {
  var geom = geojson.geometry;
  var radius = this.featureRadius();

  var coord = geojsonGetCoordinates(geojson);
  var p = pinchZoom.viewerPosFromWorldPos(coord.x, coord.y);

  if (!geojson.properties.hideIcon && geojson.properties.circled) {
    context.strokeStyle = "rgba(255,255,255,.8)";
    context.lineWidth = 5 * this.renderer.pixelRatio;
    context.beginPath();
    context.arc(p.x, p.y, radius, 0, 2 * Math.PI, false);
    context.stroke();
  }

  if (geojson.properties.text && !geojson.properties.hideText) {
    var offset = ('textOffset' in geojson.properties ?
                  geojson.properties.textOffset : 20);
    offset *= this.renderer.pixelRatio;
    var dx, dy;
    var placement = ((geojson.properties.textPlacement || 'S') + '')
      .toUpperCase();

    // horizontal settings
    switch (placement) {
      default:
      case 'C':
      case 'N':
      case 'S':
        context.textAlign = 'center';
        dx = 0;
        break;

      case 'E':
        context.textAlign = 'start';
        dx = offset;
        break;

      case 'O':
      case 'W':
        context.textAlign = 'end';
        dx = -offset;
        break;
    }

    // vertical settings
    switch (placement) {
      case 'C':
      case 'E':
      case 'W':
      case 'O':
        context.textBaseline = 'middle';
        dy = 0;
        break;

      case 'N':
        context.textBaseline = 'bottom';
        dy = -offset;
        break;

      default:
      case 'S':
        context.textBaseline = 'top';
        dy = offset;
        break;
    }

    var fontSize = 20;
    context.font = (fontSize * this.renderer.pixelRatio) + 'px '
      + geojson.properties.font || 'Roboto, "Helvetica Neue", HelveticaNeue, "Helvetica-Neue", Helvetica, Arial, "Lucida Grande", sans-serif';

    var x = p.x + dx;
    var y = p.y + dy;

    if (geojson.properties.textBubble) {
      var w = Math.ceil(context.measureText(geojson.properties.text).width);
      var h = fontSize * this.renderer.pixelRatio * 1.1;
      var bubbleX, bubbleY;
      if (context.textAlign == 'end') {
        bubbleX = x - w;
      } else if (context.textAlign == 'start') {
        bubbleX = x;
      } else {
        bubbleX = x - w /2;
      }

      if (context.textBaseline == 'top') {
        bubbleY = y;
      } else if (context.textBaseline == 'middle') {
        bubbleY = y - h/2;
      } else {
        bubbleY = y - h;
      }

      context.fillStyle = '#fff';

      var margin = 5 * this.renderer.pixelRatio;
      bubbleX = Math.round(bubbleX) - margin;
      bubbleY = Math.round(bubbleY) - margin;
      w += 2 * margin;
      h += 2 * margin;

      context.beginPath();

      context.rect(bubbleX, bubbleY, w, h);
      context.fill();

      context.moveTo(p.x, p.y);
      var dpx = -dy * .5;
      var dpy =  dx * .5;

      context.lineTo(p.x + dx + dpx, p.y + dy + dpy);
      context.lineTo(p.x + dx - dpx, p.y + dy - dpy);
      context.lineTo(p.x, p.y);

      context.fill();

    } else {
      context.strokeStyle = geojson.properties.stroke || 'rgba(255,255,255,.8)';
      context.lineWidth = 4 * this.renderer.pixelRatio;
      context.strokeText(geojson.properties.text, x, y);
    }
    context.fillStyle = geojson.properties.fill || 'rgba(0,0,0,1)';
    context.fillText(geojson.properties.text, x, y);

  }

  if (!geojson.properties.hideIcon && geojson.properties.icon && this.icons[geojson.properties.icon]) {
    var icon = this.icons[geojson.properties.icon];
    var width = (icon.width ? icon.width * this.renderer.pixelRatio : radius * 2);
    var height = (icon.height ? icon.height * this.renderer.pixelRatio : width);
    var ratioX = icon.ratioX || .5;
    var ratioY = icon.ratioY || .5;
    context.drawImage(icon.icon,
                      p.x - width * ratioX,
                      p.y - height * ratioY,
                      width, height);
  }

};

var geojsonToView = function(pinchZoom, point) {
  return pinchZoom.viewerPosFromWorldPos(Utils.latLonToWorld(point));
};

function renderPath(context, pinchZoom, points, closed) {
  var start = geojsonToView(pinchZoom, points[0]);
  context.beginPath();
  context.moveTo(start.x, start.y);
  for (var i = 1; i < points.length; ++i) {
    var p = geojsonToView(pinchZoom, points[i]);
    context.lineTo(p.x, p.y);
  }
  if (closed) {
    context.closePath();
  }
}

POILayer.prototype.renderPolygon = function(canvas, pinchZoom, feature, context) {
  context.strokeStyle = feature.properties.stroke || '#000000';
  context.lineWidth =
    feature.properties['stroke-width'] * this.renderer.pixelRatio;
  context.fillStyle = feature.properties.fill || '#ffffff';

  for (var j = 0; j < feature.geometry.coordinates.length; ++j) {
    var connectedPoly = feature.geometry.coordinates[j];
    renderPath(context, pinchZoom, connectedPoly, true);
    context.fill();
    context.stroke();
  }
};

POILayer.prototype.renderLineString =
    function(canvas, pinchZoom, feature, context) {
  context.lineWidth =
    (feature.properties['stroke-width'] || 5) * this.renderer.pixelRatio;
  context.strokeStyle = feature.properties.stroke || '#000000';
  context.lineCap = 'round';

  renderPath(context, pinchZoom, feature.geometry.coordinates, false);
  context.stroke();
};


POILayer.prototype.handleClic = function(pos) {
  var bestDist =
    this.renderer.pinchZoom.worldDistanceFromViewerDistance(this.featureRadius());
  var bestFeature = undefined;

  var t = this;

  forEachFeature(this.params.geojson, function(feature) {
    if (feature.geometry.type != 'Point' || feature.properties.hideIcon) {
      return;
    }
    var featureCoord = geojsonGetCoordinates(feature);
    var d = Utils.distance(featureCoord, pos.startWorldPos);
    if (bestDist == undefined || d < bestDist) {
      bestDist = d;
      bestFeature = feature;
    }
  });
  if(typeof bestFeature !== undefined  && bestFeature && bestFeature != null) {
    t.params.onFeatureClic(bestFeature, pos);
    return true;
  }
  return false;
};

/* options is an optional object with the following entries:
    width: icon width on map. Default: radius * 2.
    height: icon height on map. Default: same as width;
    ratioX: where the icon points to, as a ratio of the width. 0 corresponds to
            the left side, 1 to the right side. Default: 0.5.
    ratioY: where the icon points to, as a ratio of the height. 0 corresponds to
            the upper side, 1 to the lower side. Default: 0.5.
*/
POILayer.prototype.loadIcon = function(name, url, options) {
  if (name in this.icons) {
    return this.icons[name];
  }

  var icon = new Image();
  icon.src = url;
  var me = this;
  icon.onload = function() {
    me.renderer.refreshIfNotMoving();
    var obj = {
      icon: icon,
    };
    for (var i in options) {
      obj[i] = options[i];
    }
    me.icons[name] = obj;
  }
  icon.onerror = function() {
    console.log(name + ": can't load icon from: " + url);
  }
  
  return icon;
}

function getPathVisibleCoordinates(points, array) {
  for (var i = 0; i < points.length; ++i) {
    array.push(Utils.latLonToWorld(points[i]));
  }
}

POILayer.prototype.visibleCoordinateArray = function() {
  var me = this;

  var result = [];

  var f = {
    'Point' : function(feature, result) {
      if (!('hideIcon' in feature.properties) || !feature.properties.hideIcon) {
        result.push(geojsonGetCoordinates(feature));
      }
    },
    'Polygon' : function(feature, result) {
      for (var j = 0; j < feature.geometry.coordinates.length; ++j) {
        getPathVisibleCoordinates(feature.geometry.coordinates[j], result);
      }
    },
    'LineString': function(feature, result) {
      getPathVisibleCoordinates(feature.geometry.coordinates, result);
    },
  };

  forEachFeature(me.params.geojson, function(feature) {
    var type = feature.geometry && feature.geometry.type;
    if (type && f[type]) {
      f[type](feature, result);
    }
  });

  return result;
};

function boundingBox(coords) {
  var result = {
    min: { x: undefined, y: undefined},
    max: { x: undefined, y: undefined}
  };
  if (coords.length == 0) {
    return result;
  }
  result.min.x = result.max.x = coords[0].x;
  result.min.y = result.max.y = coords[0].y;
  for (var i = 1; i < coords.length; ++i) {
    result.min.x = Math.min(result.min.x, coords[i].x);
    result.min.y = Math.min(result.min.y, coords[i].y);
    result.max.x = Math.max(result.max.x, coords[i].x);
    result.max.y = Math.max(result.max.y, coords[i].y);
  }
  return result;
}

POILayer.prototype.getVisibleFeatureLocation = function(minScale, margin) {
  var bbox = boundingBox(this.visibleCoordinateArray());
  var ratio = this.renderer.canvas.width / this.renderer.canvas.height;
  var width = bbox.max.x - bbox.min.x;
  var height = bbox.max.y - bbox.min.y;
  margin = margin || 1.1;
  return {
    x: width / 2 + bbox.min.x,
    y: height / 2 + bbox.min.y,
    scale: Math.max(width * margin, height * ratio * margin, minScale)
  };
};

POILayer.prototype.zoomOnVisibleFeatures = function(minScale, margin) {
  this.renderer.setLocation(this.getVisibleFeatureLocation(minScale, margin));
};

POILayer.prototype.setGeojson = function(geojson) {
  this.params.geojson = geojson;
  this.renderer.refreshIfNotMoving();
};

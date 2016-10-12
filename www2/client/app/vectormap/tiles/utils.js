this.Utils = {
  assert: function (condition) {
    if (!condition) {
      throw('Assertion failed.');
    }
  },
  
  objectToString: function(o) {
    if (typeof o === 'object') {
      var str = '{';
      for (var i in o) {
        str += i + ': ' + Utils.objectToString(o[i]) + ',';
      }
      str += '}';
      return str;
    } else {
      return '' + o;
    }
  },

  invert3x3Matrix: function(a) {
    var a00 = a[0], a01 = a[1], a02 = a[2],
        a10 = a[3], a11 = a[4], a12 = a[5],
        a20 = a[6], a21 = a[7], a22 = a[8],

        b01 = a22 * a11 - a12 * a21,
        b11 = -a22 * a10 + a12 * a20,
        b21 = a21 * a10 - a11 * a20,

        // Calculate the determinant
        det = a00 * b01 + a01 * b11 + a02 * b21;

    if (!det) { 
        return null; 
    }
    det = 1.0 / det;

    var out = [];
    out[0] = b01 * det;
    out[1] = (-a22 * a01 + a02 * a21) * det;
    out[2] = (a12 * a01 - a02 * a11) * det;
    out[3] = b11 * det;
    out[4] = (a22 * a00 - a02 * a20) * det;
    out[5] = (-a12 * a00 + a02 * a10) * det;
    out[6] = b21 * det;
    out[7] = (-a21 * a00 + a01 * a20) * det;
    out[8] = (a11 * a00 - a01 * a10) * det;
    return out;
  },
  
  multiply3x3MatrixWithVector: function(A, b) {
    var r = [0, 0, 0];
    for (var i = 0; i < 3; ++i) {
      r[i] = A[i*3 + 0] * b[0] + A[i*3 + 1] * b[1] + A[i*3 + 2] * b[2];
    }
    return r;
  },

  multiply3x3Matrices: function(A, B) {
    var r = new Array(9);
    for (var i = 0; i < 3; ++i) {
      for (var j = 0; j < 3; ++j) {
        r[i * 3 + j] = A[i*3 + 0] * B[0 * 3 + j]
          + A[i*3 + 1] * B[1 * 3 + j]
          + A[i*3 + 2] * B[2 * 3 + j];
      }
    }
    return r;
  },
  
  distance: function(a, b) {
    var dx = a.x - b.x;
    var dy = a.y - b.y;
    return Math.sqrt(dx * dx + dy * dy);
  },

  eventPosInElementCoordinates: function(event, element) {
    var rect = element.getBoundingClientRect();
    var r = {
      x: (event.clientX - rect.left) * (element.width / element.offsetWidth),
      y: (event.clientY - rect.top) * (element.height / element.offsetHeight)
    };
    Utils.assert(!isNaN(r.x) && !isNaN(r.y));
    return r;
  },

  latLonToWorld: function(coord) {
    var lon = coord[0];
    var lat = coord[1] * Math.PI / 180;
    return {
      x: (lon + 180) / 360,
      y: ((1 - Math.log(Math.tan(lat) + 1 / Math.cos(lat)) / Math.PI) / 2)
    };
  },
  worldToLatLon: function(osm) {
    var x;
    var y;
    if (osm == undefined) {
      return undefined;
    } else if (Array.isArray(osm)) {
      x = osm[0];
      y = osm[1];
    } else if ('x' in osm && 'y' in osm) {
      x = osm.x;
      y = osm.y;
    } else {
      return undefined;
    }
    var lon_deg = x * 360.0 - 180.0;
    var n = Math.PI-2*Math.PI*y;
    var lat_deg = (180/Math.PI*Math.atan(0.5*(Math.exp(n)-Math.exp(-n))));
    return {lat: lat_deg, lon: lon_deg};
  },


  worldToTile: function(scale, coord) {
    var getTileX = function(unitX) { return  Math.floor(unitX * (1 << scale)); };
    return {
      scale: scale,
      x: getTileX(coord.x),
      y: getTileX(coord.y)
    };
  }
};

// shim layer with setTimeout fallback
window.requestAnimFrame = (function(){
  return  window.requestAnimationFrame       ||
          window.webkitRequestAnimationFrame ||
          window.mozRequestAnimationFrame    ||
          function( callback ){
            window.setTimeout(callback, 1000 / 60);
          };
})();

// creates a global "addWheelListener" method
// example: addWheelListener( elem, function( e ) { console.log( e.deltaY ); e.preventDefault(); } );
(function(window,document) {
   if (!document) {
     return;
   }

    var prefix = "", _addEventListener, onwheel, support;

    // detect event model
    if ( window.addEventListener ) {
        _addEventListener = "addEventListener";
    } else {
        _addEventListener = "attachEvent";
        prefix = "on";
    }

    // detect available wheel event
    support = "onwheel" in document.createElement("div") ? "wheel" : // Modern browsers support "wheel"
              document.onmousewheel !== undefined ? "mousewheel" : // Webkit and IE support at least "mousewheel"
              "DOMMouseScroll"; // let's assume that remaining browsers are older Firefox

    window.addWheelListener = function( elem, callback, useCapture ) {
        _addWheelListener( elem, support, callback, useCapture );

        // handle MozMousePixelScroll in older Firefox
        if( support == "DOMMouseScroll" ) {
            _addWheelListener( elem, "MozMousePixelScroll", callback, useCapture );
        }
    };

    function _addWheelListener( elem, eventName, callback, useCapture ) {
        elem[ _addEventListener ]( prefix + eventName, support == "wheel" ? callback : function( originalEvent ) {
            !originalEvent && ( originalEvent = window.event );

            // create a normalized event object
            var event = {
                // keep a ref to the original event object
                originalEvent: originalEvent,
                target: originalEvent.target || originalEvent.srcElement,
                type: "wheel",
                deltaMode: originalEvent.type == "MozMousePixelScroll" ? 0 : 1,
                deltaX: 0,
                delatZ: 0,
                pageX: originalEvent.pageX,
                pageY: originalEvent.pageY,
                clientX: originalEvent.clientX,
                clientY: originalEvent.clientY,
                preventDefault: function() {
                    originalEvent.preventDefault ?
                        originalEvent.preventDefault() :
                        originalEvent.returnValue = false;
                }
            };
            
            // calculate deltaY (and deltaX) according to the event
            if ( support == "mousewheel" ) {
                event.deltaY = - 1/40 * originalEvent.wheelDelta;
                // Webkit also support wheelDeltaX
                originalEvent.wheelDeltaX && ( event.deltaX = - 1/40 * originalEvent.wheelDeltaX );
            } else {
                event.deltaY = originalEvent.detail;
            }

            // it's time to fire the callback
            return callback( event );

        }, useCapture || false );
    }

})(window,document);

/**
 * Provides requestAnimationFrame in a cross browser way.
 * @author paulirish / http://paulirish.com/
 */
if ( !window.requestAnimationFrame ) {
    window.requestAnimationFrame = ( function() {

        return window.webkitRequestAnimationFrame ||
            window.mozRequestAnimationFrame ||
            window.oRequestAnimationFrame ||
            window.msRequestAnimationFrame ||
        function( /* function FrameRequestCallback */ callback,
                  /* DOMElement Element */ element ) {
            window.setTimeout( callback, 1000 / 60 );
        };

    } )();
};

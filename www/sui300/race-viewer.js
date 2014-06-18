this.raceViewer = null;
this.raceData = null;

function getURLParameter(name) {
  return decodeURIComponent(
    (RegExp(name + '=' + '(.+?)(&|$|#)').exec(location.search)||[,null])[1]
  );
}

$(document).on('pageinit', '#raceSelector', function () {
  var raceUrl = getURLParameter('race');
  if (raceUrl && raceUrl != "null" && raceUrl.length > 0) {
    $.mobile.navigate('#mainPage');
  }
});

function showRace(raceUrl) {
  $.ajax(raceUrl, {
    dataType: "json",
    error: function(jqXHR, textStatus, errorThrown) {
      console.log('Failed to load: '
            + raceUrl
            + ': ' + textStatus
            + ': ' + errorThrown);
    },
    success: function(data) {
      raceData = data;
      raceViewer = new RaceViewer(raceData);
      raceViewer.initialize('raceViewer');
    }
  });
}

$(document).on('pageinit', '#mainPage', function () {
  var raceUrl = getURLParameter('race');
  if (raceUrl.length > 0) {
    showRace(raceUrl);
  } else {
    $.mobile.navigate('#raceSelector');
  }
});

function RaceViewer(data) {
  this.raceData = data;
  this.playing = false;
  this.framesPerMilliseconds = .01;
  this.index = -1;
  this.scale = 1;
  this.autoCenter = false;
  this.precalc();
}

RaceViewer.prototype.updateScale = function() {
	var scale = this.scale * 
	   Math.min(this.stage.attrs.width / (this.maxX - this.minX),
                this.stage.attrs.height / (this.maxY - this.minY));
    this.layer.setScale({x: scale, y: -scale});
    this.boatOverlay.setScale({x: 1.0 / scale, y: 1.0 / scale});
}

RaceViewer.prototype.screenToWorld = function(x, y) {
	var m = this.layer.getTransform().m;
	
	var d = 1 / (m[0] * m[3] - m[1] * m[2]);
            var m0 = m[3] * d;
            var m1 = -m[1] * d;
            var m2 = -m[2] * d;
            var m3 = m[0] * d;
            var m4 = d * (m[2] * m[5] - m[3] * m[4]);
            var m5 = d * (m[1] * m[4] - m[0] * m[5]);
            
    return {
	  x: m0 * x + m2 * y + m4,
	  y: m1 * x + m3 * y + m5,
	};
}

RaceViewer.prototype.worldToScreen = function(x, y) {
	var m = this.layer.getTransform().m;
	return {
	 x: m[0] * x + m[2] * y + m[4],
	 y: m[1] * x + m[3] * y + m[5]
	};
}

RaceViewer.prototype.centerOn = function(x, y, screenX, screenY) {
	if (screenX == undefined || screenY == undefined) {
		screenX = this.stage.getWidth() / 2;
		screenY = this.stage.getHeight() / 2;
	}
	var pos = this.worldToScreen(x, y);
	this.layer.setX(this.layer.getX() + screenX - pos.x);
	this.layer.setY(this.layer.getY() + screenY - pos.y);
}

RaceViewer.prototype.resizeCanvas = function() {
  var pageHeight = $('#mainPageContainer').height();

  if (pageHeight != 0) {
    var delta = window.innerHeight - pageHeight;
    var newSize = Math.max(200, this.stage.getHeight() + delta);

    var pos = this.screenToWorld(this.stage.getWidth() / 2, this.stage.getHeight() / 2);

    if (newSize != this.stage.getHeight()) {
      this.stage.setHeight(newSize);
    }
    if (this.stage.getContainer().clientWidth != this.stage.getWidth()) {
      this.stage.setWidth(this.stage.getContainer().clientWidth);
    }
    this.updateScale();
    this.centerOn(pos.x, pos.y);
  } else {
    // Layout is not completed yet.
    var viewer = this;
    window.setTimeout(function() { viewer.refresh(); }, 100)
  }
}

RaceViewer.prototype.precalc = function() {
    // Compute bounding box.
    this.maxX = this.minX = this.raceData[0].x;
    this.maxY = this.minY = this.raceData[0].y;
    for (var i = 0; i < this.raceData.length; ++i) {
    	this.maxX = Math.max(this.maxX, this.raceData[i].x);
    	this.minX = Math.min(this.minX, this.raceData[i].x);
    	this.maxY = Math.max(this.maxY, this.raceData[i].y);
    	this.minY = Math.min(this.minY, this.raceData[i].y);
    }
    this.raceArea = {
    	width: this.maxX - this.minX,
    	height: this.maxY - this.minY,
    }
}

RaceViewer.prototype.initialize = function(containerId) {
	this.stage =  new Kinetic.Stage({
		      width: 800,
		      height: 800,
		      container: containerId,
		});
	this.layer = new Kinetic.Layer();
	this.stage.add(this.layer);
	
	this.layer.add(new Kinetic.Rect({
		x: this.minX - this.raceArea.width / 2,
		y: this.minY - this.raceArea.height / 2,
		width: this.raceArea.width * 2,
		height: this.raceArea.height * 2,
		fill: "#eef",
	}));
	this.addTrajectory();
	
        this.boatOverlay = new Kinetic.Group();
	this.layer.add(this.boatOverlay);

	this.boat = new Kinetic.Ellipse({
            radius: {x: 5, y: 5},
            x: 0,
            y: 0,
            fill: "#FF33AA"
        });
        this.boatOverlay.add(this.boat);

        this.boatOverlay.add(new Kinetic.Arc({
            innerRadius: 0,
            outerRadius: 40,
            fill: 'black',
            x: 0,
            y: 20,
            angle: 8,
            rotationDeg: -90 - 8 / 2,
        }));

        this.trueWindArrow = new Kinetic.Arc({
            innerRadius: 40,
            outerRadius: 80,
            fill: 'red',
            angle: 5,
            rotationDeg: 87.5,
            x: 0,
            y: 0
        });
        this.boatOverlay.add(this.trueWindArrow);

        this.apparentWindArrow = new Kinetic.Arc({
            innerRadius: 35,
            outerRadius: 75,
            fill: 'orange',
            angle: 5,
            rotationDeg: 87.5,
            x: 0,
            y: 0
        });
        this.boatOverlay.add(this.apparentWindArrow);

	this.layer.setDraggable(true);
	
	var currentFrameReal = 0;
	var viewer = this;
	this.animation = new Kinetic.Animation(function(frame) {
	  var deltaFrames = viewer.framesPerMilliseconds * frame.timeDiff;
      currentFrameReal += deltaFrames;
      var intPart = Math.floor(Math.abs(currentFrameReal));
      if (intPart != 0) {
      	var indexDelta = (currentFrameReal > 0 ? intPart : - intPart);
      	viewer.setCurrentPos(viewer.index + indexDelta);
      	currentFrameReal -= indexDelta;
      }
    }, [this.layer]);
    
    
    $('#timeSlider').bind('change', function(event, ui) {
    	
    	var targetPos = Math.floor(viewer.index * 1000 / viewer.raceData.length);
    	var currentPos = $('#timeSlider').val();
    	if (targetPos != currentPos) {
    		viewer.setCurrentPos(currentPos * viewer.raceData.length / 1000);
    		viewer.refresh();
    	}
    });
    
    this.updateScale();
    this.resizeCanvas();
    this.centerOn((this.maxX - this.minX) / 2, (this.maxY - this.minY) / 2);

    window.addEventListener('resize', function() { viewer.refresh(); }, false);
    document.addEventListener("mousewheel", function(event) {
    	event.preventDefault();
    	var pos = viewer.screenToWorld(event.offsetX, event.offsetY);
    	viewer.scale -= event.wheelDeltaY * .001;
    	viewer.updateScale();
    	viewer.centerOn(pos.x, pos.y, event.offsetX, event.offsetY);
    	viewer.refresh();
    }, true)

    this.stage.getContent().addEventListener('touchmove', function(evt) {
        var touch1 = evt.touches[0];
        var touch2 = evt.touches[1];

        if(touch1 && touch2) {
          var dist = getDistance({
              x: touch1.clientX,
              y: touch1.clientY
            }, {
              x: touch2.clientX,
              y: touch2.clientY
            });

            if(!lastDist) {
              lastDist = dist;
            }

            viewer.scale *= dist / lastDist;
            var scale = stage.getScale().x * dist / lastDist;
            viewer.updateScale();
            viewer.centerOn(pos.x, pos.y, event.offsetX, event.offsetY);
            viewer.refresh();

            lastDist = dist;
        }
    }, false);

    this.stage.getContent().addEventListener('touchend', function() {
        lastDist = 0;
    }, false); 
    this.setCurrentPos(0);
    viewer.refresh();
}

RaceViewer.prototype.setCurrentPos = function(index) {
    index = Math.floor(index);
    
    if (index == this.index) {
      return;
    }

    if (index >= this.raceData.length) {
      index = 0;
    }

    if (index < 0) {
      index = this.raceData.length - 1;
    }
    
    this.current = this.raceData[index];
    this.index = index;
    
    var format = {
        time: function(t) { return '' + new Date(t * 1000); },
        speedRatio: function(s) { return s.toFixed(0) + '%'; },
        awa: function(a) { return (a > 180 ? a - 360 : a).toFixed(1); },
        twa: function(a) { return (a > 180 ? a - 360 : a).toFixed(1); },
    }
    var data = this.current;
    data['twdir'] = data['twa'] + data['magHdg'];
    while (data.twdir > 360) data.twdir -= 360;

    for (var k in this.current) {	
      $("#current" + k).html(
          k in format ? format[k](data[k]) : (data[k]).toFixed(1));
    }
    
    this.boatOverlay.setRotation(-this.current.magHdg);
    this.boatOverlay.position(this.current);
    
    if (this.autoCenter) {
      this.centerOn(this.current.x, this.current.y);
    }
  
    $('#timeSlider').val(Math.floor(index * 1000 / this.raceData.length));
    $('#timeSlider').slider('refresh');
    
    this.orientWind();
}

RaceViewer.prototype.playStop = function() {
	this.playing = !this.playing;
	if (this.playing) {
		$("#playStopButton .ui-btn-text").text("Stop");
		this.animation.start();
	} else {
		$("#playStopButton .ui-btn-text").text("Play");
		this.animation.stop();
	}
}

RaceViewer.prototype.refresh = function() {
	this.resizeCanvas();
	this.layer.draw();
}

RaceViewer.prototype.faster = function() {
	this.framesPerMilliseconds *= 2;
}

RaceViewer.prototype.slower = function() {
	this.framesPerMilliseconds /= 2;
}

RaceViewer.prototype.reverse = function() {
	this.framesPerMilliseconds *= -1;
}

RaceViewer.prototype.addTrajectory = function() {
	var viewer = this;
	var trajectory = new Kinetic.Shape({
		x: 0,
		y: 0,
		stroke: 'black',
		strokeWidth: 8,
		drawFunc: function(context) {
			var data = viewer.raceData;
			context.lineWidth = 5;
			context.beginPath();
			context.moveTo(data[0].x, data[0].y);
			for (var i = 0; i < data.length; ++i) {
				context.lineTo(data[i].x, data[i].y);
			}
			context.fillStrokeShape(this);
		}
	});
	this.layer.add(trajectory);
}

RaceViewer.prototype.orientWind = function() {
	var sumSinTwa = 0;
	var sumCosTwa = 0;
	var sumSinAwa = 0;
	var sumCosAwa = 0;
	var sumWeight = 0;
	var weight = [ 1, .5, .2];
	for (var i = -1; i <= 1; ++i) {
		var index = Math.min(this.raceData.length - 1, Math.max(0, this.index + i));
		var data = this.raceData[index];
		var angle = (data.twa) * Math.PI / 180.0;
		var w = weight[Math.abs(i)];
		sumSinTwa += Math.sin(angle) * w;
		sumCosTwa += Math.cos(angle) * w;

                angle = data.awa * Math.PI / 180.0;
		sumSinAwa += Math.sin(angle) * w;
		sumCosAwa += Math.cos(angle) * w;
		sumWeight += w;
	}
	var twa = Math.atan2(sumSinTwa / w, sumCosTwa / w) * 180.0 / Math.PI;
	var awa = Math.atan2(sumSinAwa / w, sumCosAwa / w) * 180.0 / Math.PI;


        this.trueWindArrow.setRotationDeg(
            -twa + 90 - this.trueWindArrow.angle());
        this.apparentWindArrow.setRotationDeg(
            -awa + 90 - this.apparentWindArrow.angle());
}

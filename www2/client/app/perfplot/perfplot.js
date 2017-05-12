/* A class to display boat data as a graph.
 * Inspired from the following D3 example:
 * http://mbostock.github.io/d3/talk/20111018/area-gradient.html
 */
function Graph(container, fetchTile) {
  this.container = container;
  this.times = [];
  this.prepare();
  this.onZoom = undefined;

  this.loader = new DynLoader(fetchTile);
}

Graph.prototype.width = function() {
  return this.container.clientWidth;
}

Graph.prototype.height = function() {
  return this.container.clientHeight;
}

Graph.prototype.prepare = function() {
  var me = this;
  var width = this.width();
  var height = this.height();

  if (width < 40 || height < 40) {
    return false;
  }

  this.preparedWidth = width;
  this.preparedHeight = height;

  var horizontalMargin = 20;
  var verticalMargin = 20;
  var innerWidth = width - 2 * horizontalMargin;
  var innerHeight = height - 2 * verticalMargin;

  // Scales. Note the inverted domain for the y-scale: bigger is up!
  if (!this.x) {
    this.x = d3.time.scale();
  }
  var x = this.x;
  x.range([0, innerWidth]);

  if (!this.y) {
    this.y = d3.scale.linear();
  }
  var y = this.y;
  y.range([innerHeight, 0]);

  this.xAxis = d3.svg.axis()
    .scale(x)
    .orient("bottom")
    .tickSize(-innerHeight, 0)
    .tickPadding(6)
    .ticks(width / 100);
  this.yAxis = d3.svg.axis()
    .scale(y)
    .orient("right")
    .tickSize(-innerWidth)
    .tickPadding(4)
    .ticks(5);

  var fmtFunc = {
  };

  var rangeMin = function(a) { return Math.min(a[0], a[1]); };
  var rangeMax = function(a) { return Math.max(a[0], a[1]); };

  var rangeSize = function(a) {
    return Math.max(a[0], a[1]) - Math.min(a[0], a[1]);
  };

  var me = this;

  if (!this.zoom) {
    this.zoom = d3.behavior.zoom().on("zoom", function() {
      if (me.onZoom) { me.onZoom(); }
      setTimeout(function() { me.draw(); }, 0);
    });
  }

  var yForPoint = function(d) { return y(d.value); };
  if (!this.svg) {
    // Create the SVG and all its visual element (without data yet).

    // An area generator.
    this.area = d3.svg.area()
      .interpolate("linear")
      .x(function(d) { return x(d.time); })
      .y0(rangeMax(y.range()))
      .y1(yForPoint)
      .defined(function(d) { return !isNaN(d.value); });

    // A line generator.
    this.line = d3.svg.line()
      .interpolate("linear")
      .x(function(d) { return x(d.time); })
      .y(yForPoint)
      .defined(function(d) { return !isNaN(d.value); });

    this.lowLine = d3.svg.line()
      .interpolate("linear")
      .x(function(d) { return x(d.time); })
      .y(function(d) { return y(d.low); })
      .defined(function(d) { return !isNaN(d.low); });

    this.highLine = d3.svg.line()
      .interpolate("linear")
      .x(function(d) { return x(d.time); })
      .y(function(d) { return y(d.high); })
      .defined(function(d) { return !isNaN(d.high); });

    $(this.container).empty();

    this.svgRoot = d3.select(this.container).append("svg:svg")
      .attr("width", width)
      .attr("height", height);

    var svg = this.svg = this.svgRoot
      .append("svg:g")
      .attr("transform", "translate(" + horizontalMargin + "," + verticalMargin + ")");

    var gradient = svg.append("svg:defs").append("svg:linearGradient")
      .attr("id", "gradient")
      .attr("x2", "0%")
      .attr("y2", "100%");

    gradient.append("svg:stop")
      .attr("offset", "0%")
      .attr("stop-color", "#fff")
      .attr("stop-opacity", .5);

    gradient.append("svg:stop")
      .attr("offset", "100%")
      .attr("stop-color", "#a24")
      .attr("stop-opacity", 1);

    this.clipRect =
    svg.append("svg:clipPath")
      .attr("id", "clip")
      .append("svg:rect")
      .attr("x", rangeMin(x.range()))
      .attr("y", rangeMin(y.range()))
      .attr("width", rangeSize(x.range()))
      .attr("height", rangeSize(y.range()));

    this.svgYAxis =
    svg.append("svg:g")
      .attr("class", "y axis")
      .attr("transform", "translate(" + innerWidth + ",0)");

    svg.append("svg:path")
      .attr("class", "area")
      .attr("clip-path", "url(#clip)")
      .style("fill", "url(#gradient)");

    this.svgXAxis = svg.append("svg:g")
      .attr("class", "x axis")
      .attr("transform", "translate(0," + innerHeight + ")");

    this.svgTimeMarks = svg.append("svg:g")
      .attr("class", "timeMarks")
      .attr("transform", "translate(0," + innerHeight + ")");

    svg.append("svg:path")
      .attr("class", "line softline")
      .attr('id', 'lowLine')
      .attr("clip-path", "url(#clip)");

    svg.append("svg:path")
      .attr("class", "line softline")
      .attr('id', 'highLine')
      .attr("clip-path", "url(#clip)");

    svg.append("svg:path")
      .attr("class", "line")
      .attr('id', 'mainLine')
      .attr("clip-path", "url(#clip)");

    this.svgRectPane =
    svg.append("svg:rect")
      .attr("class", "pane")
      .attr("width", innerWidth)
      .attr("height", innerHeight)
      .call(this.zoom)
      .on("click", function() {
         if (me.onTimeClick) {
           me.onTimeClick(me.x.invert(d3.mouse(this)[0]));
         }
       });
  } else {
    // svg already created, needs update
    this.area.y0(rangeMax(y.range()));

    this.svgRoot
      .attr("width", width)
      .attr("height", height);

    this.svg.attr("transform",
                  "translate(" + horizontalMargin + "," + verticalMargin + ")");

    this.clipRect
      .attr("x", rangeMin(x.range()))
      .attr("y", rangeMin(y.range()))
      .attr("width", rangeSize(x.range()))
      .attr("height", rangeSize(y.range()));

    this.svgYAxis.attr("transform", "translate(" + innerWidth + ",0)");
    this.svgXAxis.attr("transform", "translate(0," + innerHeight + ")");
    this.svgTimeMarks.attr("transform", "translate(0," + innerHeight + ")");

    this.svgRectPane
      .attr("width", innerWidth)
      .attr("height", innerHeight);
  }

  var timeMarkWidth = 2;
  this.setTimeMarkAttributes = function(selection) {
    var height = this.height();
    selection.attr("class","timeMark")
      .attr("x", function(t) {
            return x(t) - timeMarkWidth / 2;
            })
    .attr("y", -height)
      .attr("width", timeMarkWidth)
      .attr("height", height);
  };
  return true;
};

Graph.prototype.setTimeBounds = function(startTime, endTime) {
    this.x.domain([startTime, endTime]);
    this.zoom.x(this.x);
};

Graph.prototype.setYBounds = function(data) {
  if (data.length > 1) {
    var getField = function(d) { return d.value; };
    this.y.domain([
      Math.min(0, d3.min(data, getField)),
      d3.max(data, getField) * 1.2]);
  }
};

Graph.prototype.setTimeMarks = function(times) {
  this.times = times;
  this.draw();
};

Graph.prototype.draw = function() {
  if (this.preparedWidth != this.width()
      || this.preparedHeight != this.height()) {
    if (!this.prepare()) {
      return;
    }
  }

  this.loader.fetchTilesInView(this.x);
  if (!this.prepareTileData()) {
    return;
  }

  var svg = this.svg;
  svg.select("g.x.axis").call(this.xAxis);
  svg.select("g.y.axis").call(this.yAxis);
  if (this.hasData) {
    svg.select("path.area").attr("d", this.area);
    svg.select("#lowLine").attr("d", this.lowLine);
    svg.select("#highLine").attr("d", this.highLine);
    svg.select("#mainLine").attr("d", this.line);
  }

  var x = this.x;
  var selection = this.svg.select("g.timeMarks").selectAll("rect").data(this.times);
  this.setTimeMarkAttributes(selection);
  this.setTimeMarkAttributes(selection.enter().append("rect"));
  selection.exit().remove();
};

Graph.prototype.tileFetchSucceeded = function (z, t, data) {
  this.loader.tileFetchSucceeded(z, t, data);
  this.draw();
};

Graph.prototype.tileFetchFailed = function (z, t) {
  this.loader.tileFetchFailed(z, t);
};

Graph.prototype.prepareTileData = function() {
  var range = this.loader.tileRangeInView(this.x);

  if (!this.preparedRange
      || this.preparedRange.firstTile != range.firstTile
      || this.preparedRange.lastTile != range.lastTile
      || this.preparedRange.zoom != range.zoom) {
    var visibleTiles = this.loader.tilesInView(this.x);

    var data = [];

    var me = this;
    var dataPending = false;
    visibleTiles.forEach(function(t) {
      var tile = me.loader.getTile(t);
      if (!tile) {
        return;
      }
      if (tile.state == me.loader.states.LOADED) {
        data = data.concat(tile.data);
      } else if (tile.state == me.loader.states.LOADING
                 || tile.state == me.loader.states.QUEUED) {
        dataPending = true;
      }
    });


    if (data.length > 0) {
      this.hasData = true;
      this.setYBounds(data);
      this.svg.select("path.area").data([data]);
      this.svg.select("#mainLine").data([data]);
      this.svg.select("#lowLine").data([data]);
      this.svg.select("#highLine").data([data]);
    } else {
      this.hasData = false;
    }

    return true;
  }
  return true;
};

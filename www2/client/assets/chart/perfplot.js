/* A class to display boat data as a graph.
 * Inspired from the following D3 example:
 * http://mbostock.github.io/d3/talk/20111018/area-gradient.html
 */
function Graph(container, fetchTile) {
  this.container = container;
  this.times = [];
  this.prepare();

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
  this.preparedWidth = width;
  this.preparedHeight = height;

  var horizontalMargin = 20;
  var verticalMargin = 20;
  var innerWidth = width - 2 * horizontalMargin;
  var innerHeight = height - 2 * verticalMargin;

  // Scales. Note the inverted domain for the y-scale: bigger is up!
  this.x = d3.time.scale().range([0, innerWidth]);
  var x = this.x;

  this.y = d3.scale.linear().range([innerHeight, 0]);
  var y = this.y;

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

  var me = this;

  var yForPoint = function(d) {
    return y(d.value);
  };

  // An area generator.
  this.area = d3.svg.area()
    .interpolate("linear")
    .x(function(d) { return x(d.time); })
    .y0(y(0))
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

  var svg = this.svg = d3.select(this.container).append("svg:svg")
    .attr("width", width)
    .attr("height", height)
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

  svg.append("svg:clipPath")
    .attr("id", "clip")
    .append("svg:rect")
    .attr("x", x(0))
    .attr("y", y(1))
    .attr("width", x(1) - x(0))
    .attr("height", y(0) - y(1));

  svg.append("svg:g")
    .attr("class", "y axis")
    .attr("transform", "translate(" + innerWidth + ",0)");

  svg.append("svg:path")
    .attr("class", "area")
    .attr("clip-path", "url(#clip)")
    .style("fill", "url(#gradient)");

  svg.append("svg:g")
    .attr("class", "x axis")
    .attr("transform", "translate(0," + innerHeight + ")");

  svg.append("svg:g")
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

  this.zoom = d3.behavior.zoom().on("zoom", function() {
    setTimeout(function() { me.draw(); }, 0);
  });

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
};

Graph.prototype.setTimeBounds = function(startTime, endTime) {
    this.x.domain([startTime, endTime]);
    this.zoom.x(this.x);
};

Graph.prototype.setYBounds = function(data) {
  if (data.length > 1) {
    var me = this;
    var getField = function(d) { return d.value; };
    this.y.domain([
      Math.min(0, d3.min(data, getField)),
      d3.max(data, getField)]);
  }

  //this.draw();
};

Graph.prototype.setTimeMarks = function(times) {
  this.times = times;
  this.draw();
};

Graph.prototype.draw = function() {
  if (this.preparedWidth != this.width()
      || this.preparedHeight != this.height()) {
    this.prepare();
  }

  this.loader.fetchTilesInView(this.x);
  if (!this.prepareTileData()) {
    return;
  }

  var svg = this.svg;
  svg.select("g.x.axis").call(this.xAxis);
  svg.select("g.y.axis").call(this.yAxis);
  svg.select("path.area").attr("d", this.area);
  svg.select("#lowLine").attr("d", this.lowLine);
  svg.select("#highLine").attr("d", this.highLine);
  svg.select("#mainLine").attr("d", this.line);

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
      || this.praparedRange.firstTile != range.firstTile
      || this.praparedRange.lastTile != range.lastTile
      || this.praparedRange.zoom != range.zoom) {
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

    if (data.length == 0 && dataPending) {
      return false;
    }

    this.setYBounds(data);

    this.svg.select("path.area").data([data]);
    this.svg.select("#mainLine").data([data]);
    this.svg.select("#lowLine").data([data]);
    this.svg.select("#highLine").data([data]);

    return true;
  }
  return true;
};

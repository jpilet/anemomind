/* A class to display boat data as a graph.
 * Heavily inspired from the following D3 example:
 * http://mbostock.github.io/d3/talk/20111018/area-gradient.html
 */
function Graph(container) {
  this.container = container;
  this.times = [];
  this.hasData = false;
  this.prepare();
  this.data = undefined;
}

Graph.prototype.width = function() {
  return angular.element(this.container).width();
}

Graph.prototype.height = function() {
  return angular.element(this.container).height();
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
    'devicePerf' : perfAtPoint,
    'deviceVmg' : vmgAtPoint
  };

  this.fieldForPoint = function(d) {
    var field = me.field;
    if (field in fmtFunc) {
      return fmtFunc[field](d);
    } else if (field in d) {
      return d[field];
    }
    return 0;
  };
  var me = this;

  var yForPoint = function(d) {
    return y(me.fieldForPoint(d));
  };

  // An area generator.
  this.area = d3.svg.area()
    .interpolate("step-after")
    .x(function(d) { return x(d.time); })
    .y0(y(0))
    .y1(yForPoint);

  // A line generator.
  this.line = d3.svg.line()
    .interpolate("step-after")
    .x(function(d) { return x(d.time); })
    .y(yForPoint);

  angular.element(this.container).empty();

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
    .attr("stop-color", "#999")
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
    .attr("class", "line")
    .attr("clip-path", "url(#clip)");

  this.zoom = d3.behavior.zoom().on("zoom", function() {me.draw(); });

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

  if (this.field && this.data) {
    this.setData(this.field, this.data);
  }
};

Graph.prototype.setData = function(field, data) {
  if (!data || !data.length || data.length == 0) {
    // no data -> we do not plot anything.
    this.field = undefined;

    if (!data) {
      return;
    }
  }

  this.field = field;
  this.data = data;

  // Bind the data to our path elements.
  this.svg.select("path.area").data([data]);
  this.svg.select("path.line").data([data]);

  var me = this;
  
  // Compute bounds
  if (data.length > 1) {
    this.x.domain([data[0].time, data[data.length - 1].time]);
    this.y.domain([0, d3.max(data, function(d) { return me.fieldForPoint(d) })]);
    this.zoom.x(this.x);
  }

  this.draw();
};

Graph.prototype.setTimeMarks = function(times) {
  this.times = times;
  this.draw();
};

Graph.prototype.draw = function() {
  if (!this.field || !this.data) {
    return;
  }

  if (this.preparedWidth != this.width() || this.preparedHeight != this.height()) {
    this.prepare();
    // Prepare will call draw() again.
    return;
  }
  var svg = this.svg;
  svg.select("g.x.axis").call(this.xAxis);
  svg.select("g.y.axis").call(this.yAxis);
  svg.select("path.area").attr("d", this.area);
  svg.select("path.line").attr("d", this.line);

  var x = this.x;
  var selection = this.svg.select("g.timeMarks").selectAll("rect").data(this.times);
  this.setTimeMarkAttributes(selection);
  this.setTimeMarkAttributes(selection.enter().append("rect"));
  selection.exit().remove();
};

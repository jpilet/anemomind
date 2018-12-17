 
function TimeBar(reference, data) {
  this.reference = reference;
  this.data = data;
  this.onTimeHighlight = function(d) {
  };
 
  //A better idom for binding with resize is to debounce
  var debounce = function(fn, timeout) 
  {
    var timeoutID = -1;
    return function() {
      if (timeoutID > -1) {
        window.clearTimeout(timeoutID);
      }
      timeoutID = window.setTimeout(fn, timeout);
    }
  };

  var me = this;
  var debounced_draw = debounce(function() { me.draw(); }, 125);

  $(window).resize(debounced_draw);
  this.draw();
}

TimeBar.prototype.setData = function(data) {
  this.data = data;
  this.draw();
}

TimeBar.prototype.draw = function() {
  var reference = this.reference;
  var data = this.data;
  if (data.length ==0) return;

  $(reference).empty()

  var minDate = this.data[0].startTime;
  var maxDate = this.data[0].startTime;
  for (var i in this.data) {
    minDate = Math.min(this.data[i].startTime, minDate);
    maxDate = Math.max(this.data[i].endTime, maxDate);
  }

  //The drawing code needs to reference a responsive elements dimneions
  var width = $(reference).width();

  var labelWidth = 110;

  // var width = $(reference).empty().width(); we can chain for effeicanecy as jquery returns jquery.

  var height = 100;  // We don't want the height to be responsive.
  var rectColor = '#000';

  /*
  var timeToX = d3.scale.ordinal()
    .domain(data.map(function(d) { return d.x; }))
    .rangeRoundBands([0, width]);
  */

  var timeToX = d3.scale.linear()
    .domain([
            minDate, maxDate
            //d3.min(data.map(function(d) { return d.y; }))
            //d3.max(data.map(function(d) { return d.y; }))
            ])
    .range([labelWidth/2, width - labelWidth/2]);
  var timeToLabelX = d3.scale.linear()
    .domain([minDate, maxDate])
    .range([0, width - labelWidth]);
  
  var svg = d3.select(reference).append("svg")
    .attr("width", width)
    .attr("height", height);

  var me = this;

  svg.selectAll("rect")
    .data(data)
    .enter().append("rect")
    .attr("width", function(d) {
          return Math.max(5, timeToX(d.endTime) - timeToX(d.startTime));
          })
    .attr("x", function(d) { return timeToX(d.startTime); })
    .attr("y", 0)
    .attr("height", height / 2)
    .attr("fill", rectColor)
    .on("mouseover", function(d) {
        d3.select(this).attr("fill", '#777');
        me.onTimeHighlight(d);
        })
    .on("mouseout", function() {
        d3.select(this).attr("fill", rectColor);
        me.onTimeHighlight(undefined);
        })
    .on("click", function(d) {
        me.onSelect(d);
        })
    ;

  svg.selectAll("text")
    .data([minDate, (maxDate + minDate) / 2, maxDate])
    .enter().append("text")
    .attr("x", function(d) { return timeToX(d); })
    .attr("y", height - 10)
    .attr("font-size", "24px")
    .attr("text-anchor", "middle")
    .text(function(d) { return new Date(d).toLocaleDateString(); });

};


var row=3;
var column=2;
var width=400;
var height=400;

var container;


var createPanel = function(){

	var svgContainer = d3.select(document.getElementById("instruments_panel")).append("svg")
	.attr("width", width)
	.attr("height", height)
	.append("g").attr("transform", "translate(" + 0 + "," + 0 + ")");

	container=svgContainer;

	//compute horizontal & vertical spacing
	var hSpace=width/column;
	var vSpace=height/row;
	var hCenter=hSpace/2;
	var vCenter=vSpace/2;

	//some testing component
	var circleData = [
	{ "x_axis": width/2, "y_axis": vCenter, "radius": 20, "color" : "green" },
	{ "x_axis": hCenter, "y_axis": vSpace+vCenter, "radius": 20, "color" : "purple"},
	{ "x_axis": hSpace+hCenter, "y_axis": vSpace+vCenter, "radius": 20, "color" : "red"},
	{ "x_axis": hCenter, "y_axis": vSpace*2+vCenter, "radius": 20, "color" : "green" },
	{ "x_axis": hSpace+hCenter, "y_axis": vSpace*2+vCenter, "radius": 20, "color" : "purple"}];


	var circles = svgContainer.selectAll("circle")
	.data(circleData)
	.enter()
	.append("circle");

	var circleAttributes = circles
	.attr("cx", function (d) { return d.x_axis; })
	.attr("cy", function (d) { return d.y_axis; })
	.attr("r", function (d) { return d.radius; })
	.style("fill", function(d) { return d.color; })



	var mySquare=svgContainer.append("rect")
  	.attr("x",60)
  	.attr("y",60)
  	.attr("width",60)
    .attr("height",60)


}

function updateGraphs(){

	//This function will update the graphs according to currentTime

	container.selectAll("circle")
	.transition()
	.attr("r",3)
	.duration(0)
  	.delay(0);

	container.selectAll("circle")
	.transition()
	.attr("r",60)
	.duration(1000)
  	.delay(100);
}
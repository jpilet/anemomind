


var Circle = function(){
	alert("salut");

	var svgContainer = d3.select("body").append("svg")
	.attr("width", 200)
	.attr("height", 600);



	var jsonCircles = [
	{ "x_axis": 30, "y_axis": 30, "radius": 20, "color" : "green" },
	{ "x_axis": 70, "y_axis": 70, "radius": 20, "color" : "purple"},
	{ "x_axis": 110, "y_axis": 200, "radius": 20, "color" : "red"}];


	var circles = svgContainer.selectAll("circle")
	.data(jsonCircles)
	.enter()
	.append("circle");



	var circleAttributes = circles
	.attr("cx", function (d) { return d.x_axis; })
	.attr("cy", function (d) { return d.y_axis; })
	.attr("r", function (d) { return d.radius; })
	.style("fill", function(d) { return d.color; });
}
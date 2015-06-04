/* The class displaying the instrument panel
 *
 */


function Panel(){
	this.row=3;
	this.column=2
	this.width=400;
	this.height=400;
	this.init();
	this.deltaTransition=1000;
	this.delayTransition=100;
}


Panel.prototype.init = function(){

	var svgContainer= this.container = d3.select(document.getElementById("instruments_panel")).append("svg")
	.attr("width", this.width)
	.attr("height", this.height);

	//compute horizontal & vertical spacing
	var hSpace=this.width/this.column;
	var vSpace=this.height/this.row;
	var hCenter=hSpace/2;
	var vCenter=vSpace/2;

	//some testing component
	var circleData = [
	{ "x_axis": this.width/2, "y_axis": vCenter, "radius": 20, "color" : "green" },
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

Panel.prototype.updatePanelGraphs = function(currentTime){

	//update the graphs according to currentTime of the scope
	this.container.selectAll("circle")
	.transition()
	.attr("r",3)
	.duration(0)
  	.delay(0);

	this.container.selectAll("circle")
	.transition()
	.attr("r",60)
	.duration(this.deltaTransition)
  	.delay(this.delayTransition);
}
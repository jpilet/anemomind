/* The class displays the instrument panel
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

	var panel = document.getElementById("instruments_panel");

	var svgContainer= this.container = d3.select(panel).append("svg")
	.attr("width", 400)
	.attr("height", 400);

	//compute horizontal & vertical spacing
	var hSpace=this.width/this.column;
	var vSpace=this.height/this.row;
	var hCenter=hSpace/2;
	var vCenter=vSpace/2;

	//some text component

	var textGroup = svgContainer.append("g");

	var perftext=svgContainer.append("text")
    .attr("x", this.width/2)
    .attr("y", vCenter)
    .attr("font-family","sans-serif")
    .attr("font-size", "30")
    .attr("fill", "black")
    .attr("id", "perf")
    .text("");

    var perftext=svgContainer.append("text")
    .attr("x", this.width/2+40)
    .attr("y", vCenter)
    .attr("font-family","sans-serif")
    .attr("font-size", "10")
    .attr("fill", "black")
    .attr("id", "percent")
    .text("%");

    var perftext=svgContainer.append("text")
    .attr("x", this.width/2)
    .attr("y", vCenter+20)
    .attr("font-family","sans-serif")
    .attr("font-size", "18")
    .attr("fill", "black")
    .attr("id", "performance")
    .text("Performance");


    /*d3.xml("/svg/gauge1.svg", "image/svg+xml", function(xml) {
    var importedNode = document.importNode(xml.documentElement, true);
    d3.select(panel).node().appendChild(importedNode);

    });*/


	var circleData = [
	/*{ "x_axis": this.width/2, "y_axis": vCenter, "radius": 20, "color" : "green" },*/
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
	.style("fill", function(d) { return d.color; });


}

Panel.prototype.updatePanelGraphs = function(currentPoint){

	if(currentPoint){
		//update perf
		this.container.selectAll("#perf")
		.text(currentPoint.devicePerf);

		//update the graphs according to currentPoint in the scope
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
}
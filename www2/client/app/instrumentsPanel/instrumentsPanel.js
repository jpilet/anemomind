/* The class displays and automate the gauge component of the instrument panel
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

	/*var svgContainer= this.container = d3.select(panel).append("svg")
	.attr("width", 200)
	.attr("height", 200);
*/

    d3.xml("assets/images/svg/gauge.svg", "image/svg+xml", function(xml) {
    var importedNode = document.importNode(xml.documentElement, true);
    this.gauge1 = d3.select(panel).node().appendChild(importedNode);

    });


}

Panel.prototype.updatePanelGraphs = function(value){

	if(value){

        console.log(this.gauge1.selectAll("g#needle"));

        this.gauge1.selectAll("g#needle")
        .transition()
        .attr("transform", "rotate(45)")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

	}
}
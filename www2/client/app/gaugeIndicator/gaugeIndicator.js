/* The class displays and automate the gauge component of the instrument panel
 *
 */


function Panel(rootElement){
	this.row=3;
	this.column=2
	this.width=400;
	this.height=400;
	this.deltaTransition=1000;
	this.delayTransition=100;
    this.root=rootElement;
    this.init();
}


Panel.prototype.init = function(){

	var panel = this.root[0];

	/*var svgContainer= this.container = d3.select(panel).append("svg")
	.attr("width", 200)
	.attr("height", 200);
*/
    var panel_component=this;

    d3.xml("assets/images/svg/gauge.svg", "image/svg+xml", function(xml) {
    var importedNode = document.importNode(xml.documentElement, true);
    panel_component.gauge1 = d3.select(panel).selectAll("#gauge-svg-container").node().appendChild(importedNode);

    });


}

Panel.prototype.updatePanelGraphs = function(value){

	if(value){
        //console.log(this.gauge1.selectAll("g#needle"));

        d3.select(this.gauge1).selectAll("#needle_inside")
        .transition()
        .attr("transform", "rotate(" + value + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

	}
}
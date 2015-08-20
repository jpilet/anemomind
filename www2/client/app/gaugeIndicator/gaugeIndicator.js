/* The class displays and automate the gauge component of the instrument panel
 *
 */


function GaugePanel(rootElement){
    this.deltaTransition=200;
    this.delayTransition=40;
    this.root = rootElement;
    this.value=0;
    this.init();
}


GaugePanel.prototype.init = function(){
	var panel = this.root[0];
    var panel_component=this;

    d3.xml("/assets/images/instruments/gauge.svg", "image/svg+xml", function(xml) {
    var importedNode = document.importNode(xml.documentElement, true);
    panel_component.gauge1 = d3.select(panel).selectAll("#gauge-svg-container").node().appendChild(importedNode);
    panel_component.updatePanelGraphs(panel_component.value);
    });


}

GaugePanel.prototype.updatePanelGraphs = function(value){

	if(value != undefined && !isNaN(value)){
      // In case the SVG has not been loaded yet, remember the value
      // so that we can apply the rotation after loading.
        this.value = value;
        d3.select(this.gauge1).selectAll("#needle_inside")
        .transition()
        .attr("transform", "rotate(" + value + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

	}
}

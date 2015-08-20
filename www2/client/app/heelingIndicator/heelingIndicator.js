/* The class displays and automate the heeling component of the instrument panel
 *
 */

function HeelingPanel(rootElement){
    this.deltaTransition=200;
    this.delayTransition=40;
    this.root=rootElement;
    this.value = 0;
    this.init();
}

HeelingPanel.prototype.init = function(){

    var panel = this.root[0];

    var panel_component=this;

    d3.xml("/assets/images/instruments/heeling.svg", "image/svg+xml", function(xml) {
    var importedNode = document.importNode(xml.documentElement, true);
    panel_component.arrow = d3.select(panel).selectAll("#heeling-svg-container").node().appendChild(importedNode);
    panel_component.updatePanelGraphs(panel_component.value);
    });
}

HeelingPanel.prototype.updatePanelGraphs = function(value){

    if(value != undefined && !isNaN(value)){
      // In case the SVG has not been loaded yet, remember the value
      // so that we can apply the rotation after loading.
        this.value = value;
        d3.select(this.arrow).selectAll("#boat")
        .transition()
        .ease("elastic")
        .attr("transform", "rotate(" + value + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

    }
}

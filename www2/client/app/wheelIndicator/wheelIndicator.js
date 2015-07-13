/* The class displays and automate the gauge component of the instrument panel
 *
 */


function WheelPanel(rootElement){
    this.width=400;
    this.height=400;
    this.deltaTransition=1000;
    this.delayTransition=100;
    this.root=rootElement;
    this.init();
}


WheelPanel.prototype.init = function(){

    var panel = this.root[0];

    var panel_component=this;

    d3.xml("assets/images/svg/wheel.svg", "image/svg+xml", function(xml) {
    var importedNode = document.importNode(xml.documentElement, true);
    panel_component.gauge1 = d3.select(panel).selectAll("#wheel-svg-container").node().appendChild(importedNode);

    });


}

WheelPanel.prototype.updatePanelGraphs = function(value){

    if(value){

        d3.select(this.gauge1).selectAll("#red")
        .transition()
        .attr("transform", "rotate(" + value + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

    }
}
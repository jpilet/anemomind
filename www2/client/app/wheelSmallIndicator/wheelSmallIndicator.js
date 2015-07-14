/* The class displays and automate the small wheel component of the instrument panel
 *
 */


function WheelSmallPanel(rootElement){
    this.width=400;
    this.height=400;
    this.deltaTransition=1000;
    this.delayTransition=100;
    this.root=rootElement;
    this.init();
}


WheelSmallPanel.prototype.init = function(){

    var panel = this.root[0];

    var panel_component=this;

    d3.xml("assets/images/svg/wheel_small.svg", "image/svg+xml", function(xml) {
    var importedNode = document.importNode(xml.documentElement, true);
    panel_component.gauge1 = d3.select(panel).selectAll("#wheel-small-svg-container").node().appendChild(importedNode);

    });


}

WheelSmallPanel.prototype.updatePanelGraphs = function(value){

    if(value){

        d3.select(this.gauge1).selectAll("#red")
        .transition()
        .attr("transform", "rotate(" + value + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

    }
}
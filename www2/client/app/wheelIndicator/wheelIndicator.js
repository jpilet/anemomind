/* The class displays and automate the wheel component of the instrument panel
 *
 */


function WheelPanel(rootElement){
    this.deltaTransition=1000;
    this.delayTransition=100;
    this.delayTextTransition=2000;
    this.root=rootElement;
    this.init();
}


WheelPanel.prototype.init = function(){

    var panel = this.root[0];

    var panel_component=this;

    d3.xml("assets/images/svg/wheel.svg", "image/svg+xml", function(xml) {
    var importedNode = document.importNode(xml.documentElement, true);
    panel_component.wheel = d3.select(panel).selectAll("#wheel-svg-container").node().appendChild(importedNode);

    });


}

WheelPanel.prototype.updatePanelGraphs = function(value, north){

    if(value && north){

        d3.select(this.wheel).selectAll("#red")
        .transition()
        .attr("transform", "rotate(" + value + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

        d3.select(this.wheel).selectAll("#north")
        .transition()
        .attr("transform", "rotate(" + north + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

        d3.select(this.wheel).selectAll("#northtext")
        .transition()
        .attr("transform", "rotate(" + north + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

    }
}

WheelPanel.prototype.updatePanelText = function(text) {

    d3.select(this.wheel).selectAll("#anglevalue")
    .transition().duration(this.delayTextTransition/2)
    .style("opacity", 0)
    .transition().duration(this.delayTextTransition/2)
    .style("opacity", 1)
    .text(text+"°");
}
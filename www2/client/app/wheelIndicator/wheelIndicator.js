/* The class displays and automate the wheel component of the instrument panel
 *
 */


 function WheelPanel(rootElement){
    this.deltaTransition=200;
    this.delayTransition=40;
    this.delayTextTransition=1000;
    this.root=rootElement;
    this.value = 0;
    this.north = 0;
    this.text = "0";
    this.init();
}


WheelPanel.prototype.init = function(){
    var panel = this.root[0];
    var panel_component=this;
    d3.xml("assets/images/svg/wheel.svg", "image/svg+xml", function(xml) {
        var importedNode = document.importNode(xml.documentElement, true);
        panel_component.wheel = d3.select(panel).selectAll("#wheel-svg-container").node().appendChild(importedNode);
    });
    // The SVG just got loaded. Rotate the arrow to where it should.
    panel_component.updatePanelGraphs(panel_component.value, panel_component.north);
    panel_component.updatePanelText(panel_component.text);


}

WheelPanel.prototype.updatePanelGraphs = function(value, north){

    if(value != undefined && north != undefined){

        // In case the SVG has not been loaded yet, remember the values
        // so that we can apply the rotation after loading.
        this.value = value;
        this.north = north;

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

    if(text != null && text != undefined){
        this.text = text;
        d3.select(this.wheel).selectAll("#anglevalue")
        .transition().duration(this.delayTextTransition/2)
        .style("opacity", 0)
        .transition().duration(this.delayTextTransition/2)
        .style("opacity", 1)
        .text(text+"°");
    }
}
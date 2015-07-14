/* The class displays and automate the arrow component of the instrument panel
 *
 */


function ArrowPanel(rootElement){
    this.width=400;
    this.height=400;
    this.deltaTransition=1000;
    this.delayTransition=400;
    this.root=rootElement;
    this.init();
}


ArrowPanel.prototype.init = function(){

    var panel = this.root[0];

    var panel_component=this;

    d3.xml("assets/images/svg/arrow.svg", "image/svg+xml", function(xml) {
    var importedNode = document.importNode(xml.documentElement, true);
    panel_component.arrow = d3.select(panel).selectAll("#arrow-svg-container").node().appendChild(importedNode);

    });


}

ArrowPanel.prototype.updatePanelGraphs = function(value){

    if(value){

        d3.select(this.arrow).selectAll("#arrow")
        .transition()
        .attr("transform", "rotate(" + value + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

    }
}
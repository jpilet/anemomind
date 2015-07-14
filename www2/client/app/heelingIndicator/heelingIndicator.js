/* The class displays and automate the heeling component of the instrument panel
 *
 */


function HeelingPanel(rootElement){
    this.width=400;
    this.height=400;
    this.deltaTransition=4000;
    this.delayTransition=600;
    this.root=rootElement;
    this.init();
}


HeelingPanel.prototype.init = function(){

    var panel = this.root[0];

    var panel_component=this;

    d3.xml("assets/images/svg/heeling.svg", "image/svg+xml", function(xml) {
    var importedNode = document.importNode(xml.documentElement, true);
    panel_component.arrow = d3.select(panel).selectAll("#heeling-svg-container").node().appendChild(importedNode);

    });


}

HeelingPanel.prototype.updatePanelGraphs = function(value){

    if(value){

        d3.select(this.arrow).selectAll("#boat")
        .transition()
        .ease("elastic")
        .attr("transform", "rotate(" + value + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

    }
}
/* The class displays and automate the heeling component of the instrument panel
 *
 */

function HeelingPanel(rootElement){
    this.deltaTransition=200;
    this.delayTransition=40;
    this.root=rootElement;
    this.value = 0;
    this.heelingSvg = d3.select(rootElement[0]).selectAll('#heelingSvg');
}

HeelingPanel.prototype.updatePanelGraphs = function(value){

    if(value != undefined && !isNaN(value)){
        this.value = value;
        this.heelingSvg.selectAll("#boat")
        .transition()
        .ease("elastic")
        .attr("transform", "rotate(" + value + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);
    }
}

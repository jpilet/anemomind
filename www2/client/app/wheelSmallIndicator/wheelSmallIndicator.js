/* The class displays and automate the small wheel component of the instrument panel
 *
 */


 function WheelSmallPanel(rootElement){
    this.deltaTransition=200;
    this.delayTransition=40;
    this.root=rootElement;
    this.arrow = 0;
    this.boat = 0;
    this.north = 0;
    this.wheelSmallSvg = d3.select(rootElement[0]).selectAll('#wheelSmallSvg');
}

WheelSmallPanel.prototype.updatePanelGraphs = function(arrow, boat, north){

    if(arrow != undefined && boat != undefined && north != undefined && !isNaN(arrow) && !isNaN(boat) && !isNaN(north)){
        this.arrow = arrow;
        this.boat = boat;
        this.north = north;

        this.wheelSmallSvg.selectAll("#red")
        .transition()
        .attr("transform", "rotate(" + arrow + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

        this.wheelSmallSvg.selectAll("#boat")
        .transition()
        .attr("transform", "rotate(" + boat + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

        this.wheelSmallSvg.selectAll("#north")
        .transition()
        .attr("transform", "rotate(" + north + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

        this.wheelSmallSvg.selectAll("#northtext")
        .transition()
        .attr("transform", "rotate(" + north + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

    }
}

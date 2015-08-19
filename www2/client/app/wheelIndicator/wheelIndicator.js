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
    this.wheel = d3.select(rootElement[0]).selectAll('#wheelSvg');
}

WheelPanel.prototype.updatePanelGraphs = function(value, north){

    if(value != undefined && north != undefined && !isNaN(value) && !isNaN(north)){

        // In case the SVG has not been loaded yet, remember the values
        // so that we can apply the rotation after loading.
        this.value = value;
        this.north = north;

        this.wheel.selectAll("#red")
        .transition()
        .attr("transform", "rotate(" + value + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

        this.wheel.selectAll("#northtext")
        .transition()
        .attr("transform", "rotate(" + north + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

        this.wheel.selectAll("#letter")
        .transition()
        .attr("transform", "rotate(" + -north + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

    }
}

WheelPanel.prototype.updatePanelText = function(text) {

    if(text != null && text != undefined){
        this.text = text;
        this.wheel.selectAll("#anglevalue")
        .text(text+"°");
    }
}

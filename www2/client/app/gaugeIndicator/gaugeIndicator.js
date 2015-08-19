/* The class displays and automate the gauge component of the instrument panel
 *
 */


function GaugePanel(rootElement){
    this.deltaTransition=200;
    this.delayTransition=40;
    this.root = rootElement;
    this.value=0;
    this.gauge1 = d3.select(this.root[0]).selectAll('#gaugeSvg');
}

GaugePanel.prototype.updatePanelGraphs = function(value){

	if(value != undefined && !isNaN(value)){
      // In case the SVG has not been loaded yet, remember the value
      // so that we can apply the rotation after loading.
        this.value = value;
        this.gauge1.selectAll("#needle_inside")
        .transition()
        .attr("transform", "rotate(" + value + ")")
        .duration(this.deltaTransition)
        .delay(this.delayTransition);

	}
}

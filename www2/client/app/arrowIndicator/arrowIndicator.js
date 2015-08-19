/* The class displays and automate the arrow component of the instrument panel
 *
 */

 function ArrowPanel(rootElement){
  this.deltaTransition=200;
  this.delayTransition=40;
  this.root=rootElement;
  this.value = 0;
  this.arrow = d3.select(rootElement[0]).selectAll('#arrowSvg');
}

ArrowPanel.prototype.updatePanelGraphs = function(value){
  if(value != undefined && !isNaN(value)){
      // In case the SVG has not been loaded yet, remember the value
      // so that we can apply the rotation after loading.
      this.value = value;
      this.arrow.selectAll("#arrow")
      .transition()
      .attr("transform", "rotate(" + value + ")")
      .duration(this.deltaTransition)
      .delay(this.delayTransition);
    }
  }

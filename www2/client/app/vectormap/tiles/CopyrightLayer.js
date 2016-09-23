function CopyrightLayer(params, renderer) {
  this.renderer = renderer;
  this.params = params || {};
  this.sizeRatio = this.params.sizeRatio || .3;
  this.margin = this.params.margin || 15;
  if (typeof(this.margin) == 'number') {
    this.margin = [this.margin, this.margin];
  }
  this.text = this.params.text || '';

  this.shadowColor = params.shadowColor || 'rgba(255,255,255,.8)';
  this.textColor = params.textColor || '#000000';
}

CopyrightLayer.prototype.draw = function(canvas, pinchZoom,
                                     bboxTopLeft, bboxBottomRight) {
  var fontSize = this.params.fontSize || 8;
  var context = canvas.getContext('2d');
  var pixelRatio =  this.renderer.pixelRatio;
  context.font = (fontSize * pixelRatio) + 'px '
      + 'Roboto, "Helvetica Neue", HelveticaNeue, "Helvetica-Neue", Helvetica, Arial, "Lucida Grande", sans-serif';

  var textX = canvas.width - this.margin[0] * pixelRatio;
  var textY = canvas.height - this.margin[1] * pixelRatio;

  context.strokeStyle = this.shadowColor;
  context.lineWidth = 3 * pixelRatio;

  context.fillStyle = this.textColor;

  context.textAlign = 'end';
  context.textBaseline = 'bottom';

  // if copyright takes more than half of the screen, we do not display it.
  if (context.measureText(this.text).width < canvas.width / 2) {
    context.strokeText(this.text, textX, textY);
    context.fillText(this.text, textX, textY);
  }
};

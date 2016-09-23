function CopyrightLayer(params, renderer) {
  this.renderer = renderer;
  this.params = params || {};
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

  var textX = canvas.width - this.margin[0] * pixelRatio;
  var textY = canvas.height - this.margin[1] * pixelRatio;

  renderShadowedText(
      context, pixelRatio, this.text,
      textX, textY,
      {
        endOrStart: 'end',
        topOrBottom: 'bottom',
        color: this.textColor,
        shadowColor: this.shadowColor,
        fontSize: fontSize,
        measureCondition: function(measure) {
          // if copyright takes more than half of the screen, we
          // do not display it.
          return measure.width < canvas.width / 2;
        }
      }
  );
};

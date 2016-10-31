'use strict';

function ClearLayer(params) {
  this.params = params;
  this.color = params.color || '#d9d9d9';
}

ClearLayer.prototype.draw = function(canvas, pinchZoom,
                                      bboxTopLeft, bboxBottomRight) {
  var context = canvas.getContext('2d');
  context.rect(0, 0, canvas.width, canvas.height);
  context.fillStyle = this.color;
  context.fill();
};


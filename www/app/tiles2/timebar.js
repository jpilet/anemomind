 
function TimeBar(reference, data) {
  this.reference = reference;
  this.data = data;
  this.onTimeSelect = function(d) {
  };
 
  //A better idom for binding with resize is to debounce
  var debounce = function(fn, timeout) 
  {
    var timeoutID = -1;
    return function() {
      if (timeoutID > -1) {
        window.clearTimeout(timeoutID);
      }
      timeoutID = window.setTimeout(fn, timeout);
    }
  };

  var me = this;
  var debounced_draw = debounce(function() { me.draw(); }, 125);

  $(window).resize(debounced_draw);
  this.draw();
}

TimeBar.prototype.setData = function(data) {
  this.data = data;
  this.draw();
}

TimeBar.prototype.draw = function() {
  var me = this;
  var reference = this.reference;
  var data = this.data;
  if (data.length ==0) return;

  $(reference).empty()

  var minDate = this.data[0].startTime;
  var maxDate = this.data[0].startTime;
  for (var i in this.data) {
    $(reference).append('<li id="' + i + '">' + this.data[i].startTime.toLocaleDateString() + '</li>');
    $('#'+i).on('mouseover', function(){
      $(this).css('background-color', 'rgba(255,0,0,0.5)');
      me.onTimeSelect(data[$(this).attr('id')]);
    });
    $('#'+i).on('mouseout', function(){
      $(this).css('background-color', '');
      me.onTimeSelect(undefined);
    });
  //   .mouseover(function() {
  //   me.onTimeSelect(data[i]);
  // }).mouseout(function(){
  //   me.onTimeSelect(undefined);
  // });
    minDate = Math.min(this.data[i].startTime, minDate);
    maxDate = Math.max(this.data[i].endTime, maxDate);
  }

  //The drawing code needs to reference a responsive elements dimneions
  var width = $(reference).width();

};


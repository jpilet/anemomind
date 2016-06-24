var DelayedCall = require('../components/DelayedCall.js');
var assert = require('assert');
var OnlineMedianFinder = require('../components/OnlineMedianFinder.js');

describe('OnlineMedianFinder', function() {
  it('omf', function() {
    var values = [ 0.9845224688760936,
                   0.9411762391682714,
                   0.4236761995125562,
                   0.12886647111736238,
                   0.4709126641973853,
                   0.38931075436994433,
                   0.16321710124611855,
                   0.16321710124611855,
                   0.16321710124611855,
                   0.7474832916632295,
                   0.15076311398297548,
                   0.35140209435485303,
                   0.6870784519705921,
                   0.01842571352608502,
                   0.2539980639703572,
                   0.5854527514893562,
                   0.7192860706709325,
                   0.8526208188850433,
                   0.8527599410153925,
                   0.3654900183901191,
                   0.8122432245872915,
                   0.7985817983280867,
                   0.4175833389163017,
                   0.04324892722070217,
                   0.7935470016673207,
                   0.4177484994288534,
                   0.06813563918694854,
                   0.5077225542627275,
                   0.2274265051819384,
                   0.46805032319389284,
                   0.9697283504065126,
                   0.29868809413164854 ];
    var finder = new OnlineMedianFinder(function(a, b) {return a - b;});

    for (var i = 0; i < values.length; i++) {
      finder.addElement(values[i]);
      if (i % 2 == 0) {
        var soFar = values.slice(0, i+1);
        soFar.sort();
        var actualMedian = soFar[Math.floor(soFar.length/2)];
        assert.equal(actualMedian, finder.currentMedian);
      }
    }
  });
});

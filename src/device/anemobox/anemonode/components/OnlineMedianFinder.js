var Heap = require('heap');
var assert = require('assert');

function lessThan(cmp, a, b) {
  return cmp(a, b) < 0;
}

function lessThanOrEqual(cmp, a, b) {
  return cmp(a, b) <= 0;
}


// Implements http://denenberg.com/omf.pdf, Figure 1
function OnlineMedianFinder(cmp) {
  this.initialized = false;
  this.cmp = cmp;
  this.currentMedian = null;
  this.smallElements = new Heap(function(a, b) {return cmp(b, a);}); // Max heap
  this.bigElements = new Heap(cmp); // Min heap
  this.balance = 0;
}

OnlineMedianFinder.prototype.addElement = function(x) {
  switch (this.balance) {
  case 0:
    if (!this.initialized || lessThan(this.cmp, x, this.currentMedian)) {
      this.smallElements.push(x);
      this.currentMedian = this.smallElements.top();
      this.balance = -1;
    } else {
      this.bigElements.push(x);
      this.currentMedian = this.bigElements.top();
      this.balance = +1;
    }
    break;
  case 1:
    if (lessThanOrEqual(this.cmp, x, this.currentMedian)) {
      this.smallElements.push(x);
    } else {
      this.bigElements.push(x);
      this.smallElements.push(this.bigElements.pop());
    }
    this.balance = 0;
    break;
  case -1:
    if (lessThanOrEqual(this.cmp, x, this.currentMedian)) {
      this.smallElements.push(x);
      this.bigElements.push(this.smallElements.pop());
    } else {
      this.bigElements.push(x);
    }
    this.balance = 0;
    break;
  };

  assert(!(this.balance == 0) || 
         this.smallElements.size() == this.bigElements.size());
  this.initialized = true;
}

OnlineMedianFinder.prototype.size = function() {
  return this.smallElements.size() + this.bigElements.size();
}

module.exports = OnlineMedianFinder;

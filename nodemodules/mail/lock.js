function Lock() {
  this.queue = [];

  // Every job is assigned a unique id by this counter.
  // This is just some safety redundancy, so that
  // we are sure that this lock is used properly.
  this.counter = 0;
  this.locked = -1;
}

/*

Example usage:

var lock = new Lock();

lock.acquire(function(release) {
  // This section is critical
  someAsyncCall(function() {
    release();
  });
});
  
  */
Lock.prototype.acquire = function(fun) {
  var wasEmpty = this.queue.length == 0;
  this.queue.push(fun);
  if (wasEmpty) {
    this.nextJob();
  }
}

// Only for internal use.
Lock.prototype.nextJob = function() {
  var job = this.queue.shift();
  if (job) {
    assert(typeof job == 'function');
    var self = this;
    this.counter++;
    var id = this.counter;
    this.locked = id;
    job(function() {
      self.release(id);
    });
  }
}

// Only for internal use.
Lock.prototype.release(id) {
  assert(this.locked == id);
  this.locked = undefined;
  this.nextJob();
}

module.exports = Lock;

'use strict';

describe('Filter: speed', function () {

  // load the filter's module
  beforeEach(module('www2App'));

  // initialize a new instance of the filter before each test
  var speed;
  beforeEach(inject(function ($filter) {
    speed = $filter('speed');
  }));

  it('should return trucated value with unit [kn]', function () {
    var text = 3.123;
    expect(speed(text)).toBe('3.1 kn');
  });
  it('should return n/a', function () {
    var text = undefined;
    expect(speed(text)).toBe('n/a');
  });
  it('should return rounded value with unit [kn]', function () {
    var text = 2.99;
    expect(speed(text)).toBe('3.0 kn');
  });
});

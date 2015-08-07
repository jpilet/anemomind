'use strict';

describe('Filter: angle', function () {

  // load the filter's module
  beforeEach(module('www2App'));

  // initialize a new instance of the filter before each test
  var angle;
  beforeEach(inject(function ($filter) {
    angle = $filter('angle');
  }));

  it('should return add 360 to a negative angle', function () {
    var text = -10;
    expect(angle(text)).toBe('350');
  });
  it('should return subtract 360 to an angle above 180', function () {
    var text = 270;
    expect(angle(text, 180)).toBe('-90');
  });
  it('should return n/a', function () {
    var text = undefined;
    expect(angle(text)).toBe('n/a');
  });

});

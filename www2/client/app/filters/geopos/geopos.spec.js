
'use strict';

describe('Filter: geopos', function () {

  // load the filter's module
  beforeEach(module('www2App'));

  // initialize a new instance of the filter before each test
  var geopos;
  beforeEach(inject(function ($filter) {
    geopos = $filter('geopos');
  }));

  it('should format a NE pos', function () {
    var pos = [0.7, 0.3];
    expect(geopos(pos)).toBe('58°13′34.616″N,71°59′60.000″E');
  });
  it('should format a SW pos', function () {
    var pos = [0.1, 0.8];
    expect(geopos(pos)).toBe('72°43′58.030″S,144°0′0.000″W');
  });

});

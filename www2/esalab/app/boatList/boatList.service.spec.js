'use strict';

describe('Service: boatList', function () {

  // load the service's module
  beforeEach(module('www2App'));

  // instantiate service
  var boatList;
  beforeEach(inject(function (_boatList_) {
    boatList = _boatList_;
  }));

  it('should do something', function () {
    expect(!!boatList).toBe(true);
  });

});

'use strict';

describe('Filter: timeSpan', function () {

  // load the filter's module
  beforeEach(module('www2App'));

  // initialize a new instance of the filter before each test
  var timeSpan;
  beforeEach(inject(function ($filter) {
    timeSpan = $filter('timeSpan');
  }));

  it('should produce a string with only hours, in local time', function () {
    var text = ['Thu Aug 25 2016 10:14:17 GMT+0200 (CEST)', 
                'Thu Aug 25 2016 12:14:17 GMT+0200 (CEST)'];
    expect(timeSpan(text)).toBe('2016-08-25 10:14 (2 hours)');
  });

  it('should produce a string with hours and days, in local time', function () {
    var text = ['Thu Aug 25 2016 10:14:17 GMT+0200 (CEST)', 
                'Thu Aug 28 2016 13:19:18 GMT+0200 (CEST)'];
    expect(timeSpan(text)).toBe('2016-08-25 10:14 (3 days, 3 hours)');
  });
  
  it('should produce a string with one day and ignore the minutes', function () {
    var text = ['Thu Aug 25 2016 10:14:17 GMT+0200 (CEST)', 
                'Thu Aug 26 2016 10:19:17 GMT+0200 (CEST)'];
    expect(timeSpan(text)).toBe('2016-08-25 10:14 (1 day)');
  });

});

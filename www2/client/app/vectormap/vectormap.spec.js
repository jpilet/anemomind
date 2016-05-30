'use strict';

describe('Controller: vectormap', function () {

  it('should encode and decode time properly', function() {
    var start = new Date("Mon May 1 2016 14:33:48 GMT+0200 (CEST)"); 
    var end = new Date("Mon May 5 2016 8:33:48 GMT+0200 (CEST)"); 
    var boat = '573ca8b0cafbf510b00d1e55';
  
    var curve = makeCurveId(boat, start, end);
    expect(curve).toEqual('573ca8b0cafbf510b00d1e552016-05-01T12:33:482016-05-05T06:33:48');

    expect(curveStartTime(curve)).toEqual(start);
    expect(curveEndTime(curve)).toEqual(end);
  });
});

'use strict';

describe('Directive: perfplot', function () {

  // load the directive's module and view
  beforeEach(module('www2App'));
  beforeEach(module('app/perfplot/perfplot.html'));

  var element, scope;

  beforeEach(inject(function ($rootScope) {
    scope = $rootScope.$new();
  }));

  it('should make hidden element visible', inject(function ($compile) {
    element = angular.element('<perfplot></perfplot>');
    element = $compile(element)(scope);
    scope.$apply();
    expect(element.text()).toBe('this is the perfplot directive');
  }));
});
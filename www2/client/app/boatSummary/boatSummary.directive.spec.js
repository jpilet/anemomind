'use strict';

describe('Directive: boatSummary', function () {

  // load the directive's module and view
  beforeEach(module('www2App'));
  beforeEach(module('app/boatSummary/boatSummary.html'));

  var element, scope;

  beforeEach(inject(function ($rootScope) {
    scope = $rootScope.$new();
  }));

  it('should make hidden element visible', inject(function ($compile) {
    element = angular.element('<boat-summary></boat-summary>');
    element = $compile(element)(scope);
    scope.$apply();
    expect(element.text()).toBe('this is the boatSummary directive');
  }));
});
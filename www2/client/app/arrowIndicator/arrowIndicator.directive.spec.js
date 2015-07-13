'use strict';

describe('Directive: arrowIndicator', function () {

  // load the directive's module and view
  beforeEach(module('www2App'));
  beforeEach(module('app/arrowIndicator/arrowIndicator.html'));

  var element, scope;

  beforeEach(inject(function ($rootScope) {
    scope = $rootScope.$new();
  }));

  it('should make hidden element visible', inject(function ($compile) {
    element = angular.element('<arrow-indicator></arrow-indicator>');
    element = $compile(element)(scope);
    scope.$apply();
    expect(element.text()).toBe('this is the arrowIndicator directive');
  }));
});
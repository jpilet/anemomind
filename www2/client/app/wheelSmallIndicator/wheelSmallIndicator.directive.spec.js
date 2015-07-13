'use strict';

describe('Directive: wheelSmallIndicator', function () {

  // load the directive's module and view
  beforeEach(module('www2App'));
  beforeEach(module('app/wheelSmallIndicator/wheelSmallIndicator.html'));

  var element, scope;

  beforeEach(inject(function ($rootScope) {
    scope = $rootScope.$new();
  }));

  it('should make hidden element visible', inject(function ($compile) {
    element = angular.element('<wheel-small-indicator></wheel-small-indicator>');
    element = $compile(element)(scope);
    scope.$apply();
    expect(element.text()).toBe('this is the wheelSmallIndicator directive');
  }));
});
'use strict';

describe('Directive: wheelIndicator', function () {

  // load the directive's module and view
  beforeEach(module('www2App'));
  beforeEach(module('app/wheelIndicator/wheelIndicator.html'));

  var element, scope;

  beforeEach(inject(function ($rootScope) {
    scope = $rootScope.$new();
  }));

  it('should make hidden element visible', inject(function ($compile) {
    element = angular.element('<wheel-indicator></wheel-indicator>');
    element = $compile(element)(scope);
    scope.$apply();
    expect(element.text()).toBe('this is the wheelIndicator directive');
  }));
});
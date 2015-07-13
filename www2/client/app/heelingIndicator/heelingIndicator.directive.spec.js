'use strict';

describe('Directive: heelingIndicator', function () {

  // load the directive's module and view
  beforeEach(module('www2App'));
  beforeEach(module('app/heelingIndicator/heelingIndicator.html'));

  var element, scope;

  beforeEach(inject(function ($rootScope) {
    scope = $rootScope.$new();
  }));

  it('should make hidden element visible', inject(function ($compile) {
    element = angular.element('<heeling-indicator></heeling-indicator>');
    element = $compile(element)(scope);
    scope.$apply();
    expect(element.text()).toBe('this is the heelingIndicator directive');
  }));
});
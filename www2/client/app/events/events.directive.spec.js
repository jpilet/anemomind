'use strict';

describe('Directive: events', function () {

  // load the directive's module and view
  beforeEach(module('www2App'));
  beforeEach(module('app/events/events.html'));

  var element, scope;

  beforeEach(inject(function ($rootScope) {
    scope = $rootScope.$new();
  }));

  it('should make hidden element visible', inject(function ($compile) {
    element = angular.element('<events></events>');
    element = $compile(element)(scope);
    scope.$apply();
    expect(element.text()).toBe('this is the events directive');
  }));
});
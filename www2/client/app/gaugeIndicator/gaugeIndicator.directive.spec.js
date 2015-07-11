'use strict';

describe('Directive: instrumentsPanel', function () {

  // load the directive's module and view
  beforeEach(module('www2App'));
  beforeEach(module('app/instrumentsPanel/instrumentsPanel.html'));

  var element, scope;

  beforeEach(inject(function ($rootScope) {
    scope = $rootScope.$new();
  }));

  it('should make hidden element visible', inject(function ($compile) {
    element = angular.element('<instruments-panel></instruments-panel>');
    element = $compile(element)(scope);
    scope.$apply();
    expect(element.text()).toBe('this is the instrumentsPanel directive');
  }));
});
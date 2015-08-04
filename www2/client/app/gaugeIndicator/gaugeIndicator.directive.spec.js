'use strict';

describe('Directive: gaugeIndicator', function () {

  // load the directive's module and view
  beforeEach(module('www2App'));
  beforeEach(module('app/gaugeIndicator/gaugeIndicator.html'));

  var element, scope;

  beforeEach(inject(function ($rootScope) {
    scope = $rootScope.$new();
  }));

  it('should change the value on the gauge', inject(function ($compile) {
    // Instanciate with a value of 63 percent.
    element = angular.element(
        '<gauge-indicator value="63" min="0" max="180" label="\'%\'" description="\"Performance\"">'
        + '</gauge-indicator>');
    element = $compile(element)(scope);
    scope.$apply();

    // gauge-indicator loads an svg asynchronously. Let's wait for the loading
    // and the animation to finish.
    waitsFor(function() {
      var array = $(element).find('#needle_inside');
      if (array.length != 1) {
        return false;
      }
      expect(array.length).toBe(1);
      var gauge = array[0];

      // Wait for the animation to finish
      if (gauge.transform.baseVal.numberOfItems == 0
          || gauge.transform.baseVal.getItem(0).angle != 63) {
        return false;
      }
      // Good. The SVG has been loaded and the angle is correct.
      return true;
    }, "The gauge can't be found or is not rotated properly", 1000);
  }));
});
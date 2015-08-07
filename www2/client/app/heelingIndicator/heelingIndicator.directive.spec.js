'use strict';

describe('Directive: heelingIndicator', function () {

  // load the directive's module and view
  beforeEach(module('www2App'));
  beforeEach(module('app/heelingIndicator/heelingIndicator.html'));

  var element, scope;

  beforeEach(inject(function ($rootScope) {
    scope = $rootScope.$new();
  }));

  it('should change the value on the heeling', inject(function ($compile) {
    // Instanciate with a value of 12 degree.
    element = angular.element(
        '<heeling-indicator value="12" label="\'deg\'" description="\"Heeling\"">'
        + '</heeling-indicator>');
    element = $compile(element)(scope);
    scope.$apply();

    // heeling-indicator loads an svg asynchronously. Let's wait for the loading
    // and the animation to finish.
    waitsFor(function() {
      var array = $(element).find('#boat');
      if (array.length != 1) {
        return false;
      }
      expect(array.length).toBe(1);
      var heeling = array[0];

      // Wait for the animation to finish
      if (heeling.transform.baseVal.numberOfItems == 0
          || Math.round(heeling.transform.baseVal.getItem(0).angle) != 12) {
        return false;
      }
      // Good. The SVG has been loaded and the angle is correct.
      return true;
    }, "The heeling can't be found or is not rotated properly", 1000);
  }));
});
'use strict';

describe('Directive: arrowIndicator', function () {

  // load the directive's module and view
  beforeEach(module('www2App'));
  beforeEach(module('app/arrowIndicator/arrowIndicator.html'));

  var element, scope;

  beforeEach(inject(function ($rootScope) {
    scope = $rootScope.$new();
  }));

  it('should rotate the arrow', inject(function ($compile) {
    // Instanciate with a rotation of 37 degrees.
    element = angular.element(
        '<arrow-indicator value="37" label="\'deg\'" description="\"TWDIR\"">'
        + '</arrow-indicator>');
    element = $compile(element)(scope);
    scope.$apply();

    // arrow-indicator loads an svg asynchronously. Let's wait for the loading
    // and the animation to finish.
    waitsFor(function() {
      var array = $(element).find('#arrow');
      if (array.length != 1) {
        return false;
      }
      expect(array.length).toBe(1);
      var arrow = array[0];

      // Wait for the animation to finish
      if (arrow.transform.baseVal.numberOfItems == 0
          || arrow.transform.baseVal.getItem(0).angle != 37) {
        return false;
      }
      // Good. The SVG has been loaded and the angle is correct.
      return true;
    }, "The arrow can't be found or is not rotated properly", 1000);
  }));
});

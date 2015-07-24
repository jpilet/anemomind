'use strict';

describe('Directive: wheelIndicator', function () {

  // load the directive's module and view
  beforeEach(module('www2App'));
  beforeEach(module('app/wheelIndicator/wheelIndicator.html'));

  var element, scope;

  beforeEach(inject(function ($rootScope) {
    scope = $rootScope.$new();
  }));

    it('should change the value of angle, north and northtext on the wheel', inject(function ($compile) {
    // Instanciate with a value of 63 degree and north value of 89 degree.
    element = angular.element(
        '<wheel-indicator arrow="63" north="89" label="\'deg\'" description="\"Wheel\"">'
        + '</wheel-indicator>');
    element = $compile(element)(scope);
    scope.$apply();

    // wheel-indicator loads an svg asynchronously. Let's wait for the loading
    // and the animation to finish.
    waitsFor(function() {
      var array = $(element).find('#red');
      if (array.length != 1) {
        return false;
      }
      expect(array.length).toBe(1);
      var wheel = array[0];

      // Wait for the animation to finish
      if (wheel.transform.baseVal.numberOfItems == 0
          || wheel.transform.baseVal.getItem(0).angle != 63) {
        return false;
      }

      //Now test the north indicator rotation
      var array1 = $(element).find('#north');
      if (array1.length != 1) {
        return false;
      }
      expect(array1.length).toBe(1);
      var wheel1 = array1[0];

      // Wait for the animation to finish
      if (wheel1.transform.baseVal.numberOfItems == 0
          || wheel1.transform.baseVal.getItem(0).angle != 89) {
        return false;
      }

      //Now test the north text rotation
      var array2 = $(element).find('#northtext');
      if (array2.length != 1) {
        return false;
      }
      expect(array2.length).toBe(1);
      var wheel2 = array2[0];

      // Wait for the animation to finish
      if (wheel2.transform.baseVal.numberOfItems == 0
          || wheel2.transform.baseVal.getItem(0).angle != 89) {
        return false;
      }

      // Good. The SVG has been loaded and the angle is correct.
      return true;
    }, "The wheel can't be found or is not rotated properly", 1000);
});
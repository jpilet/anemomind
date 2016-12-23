'use strict';

describe('Directive: wheelSmallIndicator', function () {

  // load the directive's module and view
  beforeEach(module('www2App'));
  beforeEach(module('app/wheelSmallIndicator/wheelSmallIndicator.html'));

  var element, scope;

  beforeEach(inject(function ($rootScope) {
    scope = $rootScope.$new();
  }));

  it('should change the value of angle, boat, north and northtext on the small wheel', inject(function ($compile, $httpBackend) {
    // Instanciate with a value of 63 degree and north value of 89 degree.
    element = angular.element(
        '<wheel-small-indicator arrow="77" boat="42" north="24" label="\'deg\'" description="\"small wheel\"">'
        + '</wheel-small-indicator>');
    element = $compile(element)(scope);

    // I do not understand why angular is fetching home.html.
    // But it does.
    $httpBackend.when('GET', 'app/home/home.html').respond('');
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
          || wheel.transform.baseVal.getItem(0).angle != 77) {
        return false;
      }
      //Now test the boat indicator rotation
      var array0 = $(element).find('#boat');
      if (array0.length != 1) {
        return false;
      }
      expect(array0.length).toBe(1);
      var wheel0 = array0[0];

      // Wait for the animation to finish
      if (wheel0.transform.baseVal.numberOfItems == 0
          || wheel0.transform.baseVal.getItem(0).angle != 42) {
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
          || wheel1.transform.baseVal.getItem(0).angle != 24) {
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
          || wheel2.transform.baseVal.getItem(0).angle != 24) {
        return false;
      }

      // Good. The SVG has been loaded and the angle is correct.
      return true;
    }, "The wheel can't be found or is not rotated properly", 1000);
  }));
});

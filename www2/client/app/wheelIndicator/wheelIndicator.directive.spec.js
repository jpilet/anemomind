'use strict';

describe('Directive: wheelIndicator', function () {

  // load the directive's module and view
  beforeEach(module('www2App'));
  beforeEach(module('app/wheelIndicator/wheelIndicator.html'));

  var element, scope;
  var $httpBackend;

  beforeEach(inject(function ($rootScope, $injector) {
    scope = $rootScope.$new();

    // Set up the mock http service responses
    $httpBackend = $injector.get('$httpBackend');

    // I do not understand why angular is fetching home.html.
    // But it does.
    $httpBackend.when('GET', 'app/home/home.html').respond('');
  }));

    it('should change the value of angle, north and northtext on the wheel', inject(function ($compile) {
    // Instanciate with a value of 63 degree and north value of 89 degree.
    element = angular.element(
        '<wheel-indicator arrow="63" north="89" label="\'deg\'" description="\"Wheel\"">'
        + '</wheel-indicator>');
    element = $compile(element)(scope);
 
    // I do not understand why angular is fetching home.html.
    // But it does.
    $httpBackend.when('GET', 'app/home/home.html').respond('');
    scope.$apply();

    // wheel-indicator loads an svg asynchronously. Let's wait for the loading
    // and the animation to finish.
    var wheel;
    waitsFor(function() {
      var array = $(element).find('#red');
      if (array.length != 1) {
        return false;
      }
      wheel = array[0];
      return true;
    }, 'looking for the wheel with #red tag', 1000);
    waitsFor(function() {
      // Wait for the animation to finish
      if (wheel.transform.baseVal.numberOfItems == 0
          || wheel.transform.baseVal.getItem(0).angle < 63) {
        return false;
      }
      return true;
    }, 'wheel animation to finish', 1000);

    var north;
    runs(function() {
      //Now test the north indicator rotation
      var array1 = $(element).find('#letter');
      expect(array1.length).toBe(1);
      north = array1[0];
    });

    waitsFor(function() {
      // Wait for the animation to finish
      expect(north.transform.baseVal.numberOfItems).not.toBe(0);
      if (north.transform.baseVal.numberOfItems == 0) {
        return false;
      }
      if (north.transform.baseVal.getItem(0).angle != -89) {
        return false;
      }
      return true;
    }, 'north animation to finish', 1000);

    runs(function() {
      //Now test the north text rotation
      var array2 = $(element).find('#northtext');
      expect(array2.length).toBe(1);
      var wheel2 = array2[0];

      expect(wheel2.transform.baseVal.numberOfItems).not.toBeLessThan(1);
      expect(wheel2.transform.baseVal.getItem(0).angle).toBe(89);
    });
  }));
});

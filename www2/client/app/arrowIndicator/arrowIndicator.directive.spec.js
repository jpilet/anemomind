'use strict';

describe('Directive: arrowIndicator', function () {

  // load the directive's module and view
  beforeEach(module('www2App'));
  beforeEach(module('app/arrowIndicator/arrowIndicator.html'));

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

  it('should rotate the arrow', inject(function ($compile) {
    // Instanciate with a rotation of 37 degrees.
    element = angular.element(
        '<arrow-indicator value="37" to="0" label="\'deg\'" description="\"TWDIR\"">'
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
      return true;
    }, "The arrow can't be found", 1000);

    waitsFor(function() {
      var array = $(element).find('#arrow');

      expect(array.length).toBe(1);
      var arrow = array[0];

      // Wait for the animation to finish
      if (arrow.transform.baseVal.numberOfItems == 0
          || Math.round(arrow.transform.baseVal.getItem(0).angle) != 37) {
        return false;
      }
      // Good. The SVG has been loaded and the angle is correct.
      return true;
    }, "The arrow is not rotated properly", 500);
  }));
});

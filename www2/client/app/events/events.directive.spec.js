'use strict';

describe('Directive: events', function () {

  // load the directive's module and view
  beforeEach(module('www2App'));
  beforeEach(module('app/events/events.html'));

  var element, scope;
  var httpBackend;

  beforeEach(inject(function ($rootScope, $httpBackend) {
     // Set up the mock http service responses
     httpBackend = $httpBackend;

    scope = $rootScope.$new();
  }));

  afterEach(function() {
     httpBackend.verifyNoOutstandingExpectation();
     httpBackend.verifyNoOutstandingRequest();
  });

  it('should make hidden element visible', inject(function ($compile) {
    httpBackend.expectGET('/api/events?b=testBoat').respond(200, []);
    element = angular.element('<events boat="\'testBoat\'"></events>');
    element = $compile(element)(scope);
    scope.$apply();
    httpBackend.flush();
  }));
});

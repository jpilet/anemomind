'use strict';

describe('Controller: RaceListCtrl', function () {

  // load the controller's module
  beforeEach(module('anemomindApp'));

  var RaceListCtrl;
  var scope;
  var $httpBackend;

  // Initialize the controller and a mock scope
  beforeEach(inject(function (_$httpBackend_, $controller, $rootScope) {
    $httpBackend = _$httpBackend_;
    $httpBackend.expectGET('/api/races')
      .respond(['race 1', 'race 2', 'race 3']);
    scope = $rootScope.$new();
    RaceListCtrl = $controller('RaceListCtrl', {
      $scope: scope
    });
  }));

  it('should attach a list of races to the scope', function () {
    expect(scope.races.length).toBe(0);
    $httpBackend.flush();
    expect(scope.races.length).toBe(3);
  });
});

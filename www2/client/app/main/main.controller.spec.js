'use strict';

describe('Controller: PathController', function () {

  // load the controller's module
  beforeEach(module('www2App'));
  beforeEach(module('socketMock'));

  var PathController,
      scope,
      $httpBackend;

  // Initialize the controller and a mock scope
  beforeEach(inject(function (_$httpBackend_, $controller, $rootScope) {
    $httpBackend = _$httpBackend_;
    scope = $rootScope.$new();
    PathController = $controller('PathController', {
      $scope: scope
    });
  }));
});

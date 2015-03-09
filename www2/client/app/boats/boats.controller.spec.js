'use strict';

describe('Controller: BoatsCtrl', function () {

  // load the controller's module
  beforeEach(module('www2App'));

  var BoatsCtrl, scope;

  // Initialize the controller and a mock scope
  beforeEach(inject(function ($controller, $rootScope) {
    scope = $rootScope.$new();
    BoatsCtrl = $controller('BoatsCtrl', {
      $scope: scope
    });
  }));

  it('should ...', function () {
    expect(1).toEqual(1);
  });
});

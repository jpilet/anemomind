'use strict';

describe('Service: userDB', function () {

  // load the service's module
  beforeEach(module('www2App'));

  // instantiate service
  var userDB;
  var $httpBackend;
  var userLookupHandler;

  beforeEach(inject(function (_userDB_, $injector) {
    userDB = _userDB_;

    // Set up the mock http service responses
    $httpBackend = $injector.get('$httpBackend');

    // I do not understand why angular is fetching home.html.
    // But it does.
    $httpBackend.when('GET', 'app/home/home.html').respond('');

    userLookupHandler = $httpBackend.when('GET', '/api/users/12345')
      .respond({_id: '12345', name: 'Test User'});

  }));

  it('should call the backend only the first time', function (done) {
    expect(!!userDB).toBe(true);
  
    $httpBackend.expectGET('/api/users/12345');
    userDB.resolveUser('12345', function(data) {
      expect(data._id).toBe('12345');
      expect(data.name).toBe('Test User');
    }); 
    $httpBackend.flush();
    $httpBackend.verifyNoOutstandingExpectation();
    $httpBackend.verifyNoOutstandingRequest();

    userDB.resolveUser('12345', function(data) {
      expect(data._id).toBe('12345');
      expect(data.name).toBe('Test User');
    }); 
    $httpBackend.verifyNoOutstandingExpectation();
    $httpBackend.verifyNoOutstandingRequest();
  });

  afterEach(function() {
    $httpBackend.verifyNoOutstandingExpectation();
    $httpBackend.verifyNoOutstandingRequest();
  });
});

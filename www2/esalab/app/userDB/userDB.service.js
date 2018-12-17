'use strict';

// A small cache to keep a map of userid -> userdata.
angular.module('www2App')
  .factory('userDB', function ($http) {
    var users = {};
    return {
      resolveUser: function(userId, callback) {
        if (userId in users) {
          callback(users[userId]);
        } else {
          $http.get('/api/users/' + userId)
          .success(function(data, status, headers, config) {
            if (data && data._id) {
              users[data._id] = data;
              callback(data);
            }
          });
        }
      }
    };
  });

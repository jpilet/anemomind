'use strict';

angular.module('www2App')
    .directive('userDetails', function ($http, Auth) {
        return {
            templateUrl: 'app/account/settings/userDetail.html',
            restrict: 'E',
            link: function (scope, element, attrs) {
                scope.countries = [];
                // get the list of countries
                $http.get("/api/pricing/getCountries")
                    .then(function (response) {
                        scope.countries = response.data;
                    });

                scope.user = Auth.getCurrentUser();
            }
        }
    }); 
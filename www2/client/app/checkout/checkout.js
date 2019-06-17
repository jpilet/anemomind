'use strict';

angular.module('www2App')
    .config(function ($stateProvider) {
        $stateProvider
            .state('checkout', {
                url: '/checkout:boatId',
                templateUrl: 'app/checkout/checkout.html',
                controller: 'CheckoutCtrl'
            })

    });

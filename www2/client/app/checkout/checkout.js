'use strict';

angular.module('www2App')
    .config(function ($stateProvider) {
        $stateProvider
            .state('checkout', {
                url: '/checkout/:boatId?plan',
                templateUrl: 'app/checkout/checkout.html',
                controller: 'CheckoutCtrl'
            })

    });

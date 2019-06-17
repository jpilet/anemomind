angular.module('www2App')
    .factory('Checkout', function () {
        // Default value is null
        var selectedBoat = null;
        var selectedPlan = null;

        return {
            setBoat: function (boat) {
                selectedBoat = boat;
            },

            getBoat: function () {
                return selectedBoat;
            },

            setSelectedPlan: function (plan) {
                selectedPlan = plan;
            },

            getSelectedPlan: function () {
                return selectedPlan;
            }
        }
    });
'use strict';

angular.module('www2App')
  .controller('ShareCtrl', function ($scope, $element, ModalService) {

      $scope.iconList = [
      {
        name: 'linkedin',
        url: '#'
      },
      {
        name: 'facebook',
        url: '#'
      },
      {
        name: 'pinterest',
        url: '#'
      },
      {
        name: 'twitter',
        url: '#'
      },
      {
        name: 'google',
        url: '#'
      },
      {
        name: 'mail',
        url: '#'
      }
    ];

    $scope.shareMe = ModalService.data.boat+' and her team made a great performance with an average speed of '+ModalService.data.speed+' .'+ModalService.data.speedUnit;
    $scope.shareMe += '\n\nAdd text ...';

    $scope.closeModal = function() {
      $element.hide();
    };
  });

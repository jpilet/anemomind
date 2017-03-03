'use strict';

angular.module('www2App')
  .controller('ShareCtrl', function ($scope, $stateParams, $location, $element, boatList, ModalService, Auth) {

      $scope.boat = { _id: $stateParams.boatId, name: 'loading', publicAccess:false };

      $scope.isVisible = (typeof ModalService.isVisible === 'undefined' || ModalService.isVisible == null) ? false : ModalService.isVisible;

      $scope.iconList = [
      {
        name:'facebook',
        icon:'fa-facebook',
        url: 'https://www.facebook.com/sharer/sharer.php?u=',
        use:true
      },
      {
        name: 'linkedin',
        icon:'fa-linkedin',
        url: 'https://www.linkedin.com/shareArticle?mini=true&url=',//&title=_TITLE_&summary=_TXT_'
        use:false
      },
      {
        name: 'pinterest',
        url: '#',
        use:false
      },
      {
        name: 'twitter',
        icon:'fa-twitter',
        url: 'https://twitter.com/home?status=',
        use:true
      },
      {
        name: 'google',
        icon:'fa-google-plus',
        url: 'https://plus.google.com/share?url=',
        use:true
      },
      {
        name: 'mail',
        url: '#',
        use:false
      }
    ];
    $scope.selectedIcon=$scope.iconList[0];
    

    // This is used for the modal template in Challenges page
    if(typeof ModalService.data !== 'undefined' || ModalService.data != null) {
      $scope.shareMe = ModalService.data.boat+' and her team made a great performance with an average speed of '+ModalService.data.speed+' .'+ModalService.data.speedUnit;
      $scope.shareMe += '\n\nAdd text ...';
    }
    
    // This is used for the modal template in Map page
    if(typeof $stateParams.boatId !== 'undefined' || $stateParams.boatId != null) {
      boatList.boat($stateParams.boatId).then(function (boat) {
        var aveSpeedText = '32 Kts';
        var windBlowedText = '22 Kts';
        var performanceText = '91%';
        $scope.admin = boat.admins.findIndex(function(x) {
          return x == Auth.getCurrentUser()._id; }) >= 0;
        $scope.boat = boat;
        $scope.sharedLink = $location.absUrl();
        $scope.shareText = 'Share ' + boat.name + ' and her team performance';
      });

      $scope.changePublicAccess=function() {
        $scope.boat.publicAccess = ! $scope.boat.publicAccess;      
        boatList.save($stateParams.boatId, $scope.boat)
          .success(function(boat) { 
            $scope.boat = boat; 
          });
      };
    }
    

    $scope.goShare=function(icon){
      window.open(icon.url + encodeURIComponent($scope.sharedLink), "_new");
    };

    $scope.selecteIcon=function(icon){
      $scope.selectedIcon=icon;
    };


    $scope.closeModal = function() {
      // The background of the modal
      $element.next().remove();
      $element.remove();      
    };
  });

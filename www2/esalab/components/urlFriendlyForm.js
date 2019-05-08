function urlFriendlyForm(x) {
  return (x ? x.replace(/[^~a-zA-Z0-9_.,+-]/g, '.').replace(/\.+/g, '.') : '');
};

angular.module('www2App')
.filter('urlFriendlyForm', function() { return urlFriendlyForm; });


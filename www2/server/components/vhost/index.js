
function vhostForReq(req) {
  if (process.env.VHOST) {
    const vhost = process.env.VHOST;
    if (vhost == 'esalab' || vhost == 'client') {
      return vhost;
    }
    console.warn('environment variable VHOST is set to ' + vhost
                 + ' which is not a valid virtual host');
  }

  if (req.host.match(/(esa)|(regattapolar)/)) {
    return 'esalab';
  }
  return 'client';
}

module.exports.vhostForReq = vhostForReq;

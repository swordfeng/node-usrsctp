var usrsctp = require('./lib/usrsctp.js'),
	util = require('util');

var t = new usrsctp.Socket({style:'stream', family:6}, util.inspect);
t.listen(5001, 1, function() {console.log('listening');});

setInterval(function(){}, 1000);

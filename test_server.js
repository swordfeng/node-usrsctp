var usrsctp = require('./lib/usrsctp.js');

var t = new usrsctp.Socket({style:'stream', family:6}, function(c) {
//	console.log(c);
});
t.listen(1234, 1, function() {console.log('listening');});

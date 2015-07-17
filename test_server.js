var usrsctp = require('./lib/usrsctp.js');

var t = new usrsctp.Socket({style:'stream', family:6}, function(c) {
//	console.log(c);
usrsctp._syscall.setSockRecvBufferSize(c._nativeSocket, 1);
});
t.listen(5001, 1, function() {console.log('listening');});

setInterval(function(){}, 1000);

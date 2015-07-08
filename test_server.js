var usrsctp = require('./build/Release/usrsctp');

usrsctp.init(9899);
var t = usrsctp.socket(6, "seqpacket");
console.log(t);

t.onData = function(buf, info) {
	console.log(info);
	console.log(buf.length);
};

t.onNotification = function(notif) {
	console.log(notif);
};

console.log("bind");
usrsctp.bind(t, '::', 1234, true);

console.log("listen");
usrsctp.listen(t, true);



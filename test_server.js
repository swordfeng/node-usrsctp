var usrsctp = require('./build/Release/usrsctp');

usrsctp.init(9899);
var t = usrsctp.socket(6, "seqpacket");

t.onData = function(buf, info) {
	console.log("DATA:", info);
};

t.onNotification = function(notif) {
	console.log("NOTIFICATION:", notif);
};

console.log("bind");
usrsctp.bind(t, '::', 1234, true);

console.log("listen");
usrsctp.listen(t, true);



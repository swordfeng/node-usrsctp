var usrsctp = require('./build/Release/usrsctp');

var sock = require('dgram').createSocket('udp4');
sock.bind(11111);
sock.on('message', function(){});


usrsctp.init(9899);
var t = usrsctp.socket(6, "seqpacket");
console.log(t);

t.callback = function(buf) {
	console.log(buf.toString('utf8'));
};

console.log("bind");
usrsctp.bind(t, '::1', 1234, true);

console.log("listen");
usrsctp.listen(t, true);



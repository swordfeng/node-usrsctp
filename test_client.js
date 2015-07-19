var usrsctp = require('./lib/usrsctp.js');

usrsctp.init(15000);
var sock = usrsctp.Socket({family: 6, style: "seqpacket"});
sock.on('error', console.log);
console.log(sock);
sock.connect(5001, '::ffff:127.0.0.1', function(assoc) {
	console.log(assoc);
	process.exit();
});
/*
var count = 0;
var timer = setInterval(function(){
	if (count < 2) {
		usrsctp.send(t, {assoc_id:aid, stream_id:2, ppid:1234, unordered:false}, new Buffer(131072), 0, 131072);
	} else if (count == 2) {
		//usrsctp.close(t);
	} else {
		clearInterval(timer);
	}
	count++;
},500);
*/

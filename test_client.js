var usrsctp = require('./build/Release/usrsctp');


usrsctp.init(15000);
var t = usrsctp.socket(6, "seqpacket");
//usrsctp.bind(t, '::', 5000);
var aid = usrsctp.connect(t, '::ffff:127.0.0.1', 5001, 9899);
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

process.on('exit', function() {
	console.log('exiting');
	usrsctp.finish(false);
});

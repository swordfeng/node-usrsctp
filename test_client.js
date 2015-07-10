var usrsctp = require('./build/Release/usrsctp');


usrsctp.init(10000+parseInt(Math.random()*10000));
var t = usrsctp.socket(6, "seqpacket");

var aid = usrsctp.connect(t, '::1', 1234, 9899);
var count = 0;
var timer = setInterval(function(){
	if (count < 10) {
		usrsctp.send(t, {assoc_id:aid, stream_id:2, ppid:1234, unordered:false}, new Buffer(131072), 0, 131072);
	} else if (count == 10) {
		usrsctp.close(t);
	} else {
		clearInterval(timer);
	}
	count++;
},500);

process.on('exit', function() {
	usrsctp.finish(false);
});

var usrsctp = require('./build/Release/usrsctp');


usrsctp.init(10000+parseInt(Math.random()*10000));
var t = usrsctp.socket(6, "seqpacket");
usrsctp.close(t);
t = usrsctp.socket(6, "seqpacket");
usrsctp.close(t);
t = usrsctp.socket(6, "seqpacket");
usrsctp.close(t);
t = usrsctp.socket(6, "seqpacket");
usrsctp.close(t);

usrsctp.finish(false);
setTimeout(function(){
	
//usrsctp.send(t, {assoc_id:aid, stream_id:2, ppid:1234, unordered:false}, new Buffer(131072), 0, 131072);
},2000);


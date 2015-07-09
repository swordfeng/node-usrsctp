var usrsctp = require('./build/Release/usrsctp');


usrsctp.init(10000+parseInt(Math.random()*10000));
var t = usrsctp.socket(4, "seqpacket");
console.log(t);

console.log("connect");
var aid = usrsctp.connect(t, '127.0.0.1', 1234, 9899);

console.log(aid);

var i=0;

setInterval(function(){
	
usrsctp.send(t, {assoc_id:aid, stream_id:2, ppid:1234, unordered:false}, new Buffer(131072), 0, 131072);
},200);

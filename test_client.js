var usrsctp = require('./build/Release/usrsctp');


usrsctp.init(9988);
var t = usrsctp.socket(6, "seqpacket");
console.log(t);

console.log("connect");
var aid = usrsctp.connect(t, '::1', 1234, 9899);

console.log(aid);

setInterval(function(){usrsctp.send(t, {assoc_id:aid, stream_id:0, ppid:0, context:0, flags:0}, new Buffer('hello world','utf8'), 0, 11);}, 200);




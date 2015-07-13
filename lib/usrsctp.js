var _ = require('underscore');
var usrsctp = module.exports = {
	Socket: null, // define later, same below
	Association: null,
	init: null,
	listenSIGINT: null,
	_syscall: null,
	_initialized: false,
};

var syscall = usrsctp._syscall = require('../build/Release/usrsctp.node');

var Socket = usrsctp.Socket = function() {
	if (!(this instanceof Socket)) {
		return Socket.apply(Object.create(Socket.prototype), arguments);
	}
	
	var options = {},
		connectionListener;
	if (typeof arguments[0] === 'function') {
		connectionListener = arguments[0];
	} else if (typeof arguments[1] === 'function') {
		options = arguements[0];
		connectionListener = arguments[1];
	}
	options = _.chain(options)
			.pick('family', 'style')
			.defaults({
				'family': 6,
				'style': 'seqpacket',
			});
	var family = options.family,
		style = options.style;
	if (!usrsctp._initialized) usrsctp.init(9899);
	if (family !== 4 && family !== 6) {
		throw Error('Invalid IP family. Valid values are: 4, 6');
	}
	var typevalue;
	if (style === 'seqpacket') typevalue = false;
	else if (style === 'stream') typevalue = true;
	else throw Error('Invalid socket type. Valid values are: seqpacket, stream');
	var nativeSocket = this._nativeSocket = _syscall.socket(family, typevalue);
	this._style = type;
	
	if (style === 'stream') {
		nativeSocket.onNotification  =;
	} else {
	}
};

Socket.prototype = {
	_nativeSocket: null,
	_associations: {},
	_style: null,
};

Socket.prototype.listen = function(options) {
};

Socket.prototype.connect = function(options) {
};

var Association = usrsctp.Association = function() {};

usrsctp.init = function(port) {
	if (usrsctp._initialized) {
		throw Error('Usrsctp has already been initialized');
	}
	if (!Number.isSafeInteger(port) || port < 0 || port > 65535) {
		throw Error('Invalid port number');
	}
	usrsctp._initialized = true;
	syscall.init(port);
	return usrsctp;
};

usrsctp.createServer = function() {
	return Socket.apply(Object.create(Socket.prototype), arguments);
};
usrsctp.createConnection;
usrsctp.connect = usrsctp.createConnection;

function sigintListener() {
	if (process.listeners('SIGINT').length === 1) {
		process.exit();
	}
}
function processNewListener(event, listener) {
	if (listener === sigintListener) return;
	if (event !== 'SIGINT') return;
	usrsctp.listenSIGINT(false);
}
usrsctp.listenSIGINT = function(enabled) {
	if (enabled && process.listeners('SIGINT').length === 0) {
		process.on('SIGINT', sigintListener);
		process.on('newListener', processNewListener);
	} else {
		try {
			process.removeListener('SIGINT', sigintListener);
			process.removeListener('newListener', processNewListener);
		} catch(e) {}
	}
};

process.on('exit', function() {
	syscall.finish();
});

usrsctp.listenSIGINT(true);
syscall.setRecvBufferSize(1048576);
syscall.setSendBufferSize(1048576);

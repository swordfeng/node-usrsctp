var events = require('events'),
	EventEmitter = events.EventEmitter,
	_ = require('underscore');
var usrsctp = module.exports = {
	Socket: null, // define later, same below
	Association: null,
	init: null,
	listenSIGINT: null,
	_syscall: null,
	_initialized: false,
	_activeSockets: 0,
};

var syscall = usrsctp._syscall = require('../build/Release/usrsctp.node');

var Socket = usrsctp.Socket = function() {
	if (!(this instanceof Socket)) {
		return Socket.apply(Object.create(Socket.prototype), arguments);
	}
	
	if (this._super) _.defaults(this, new this._super);
	var self = this;
	
	var options = {},
		connectionListener;
	if (typeof arguments[0] === 'function') {
		connectionListener = arguments[0];
	} else if (typeof arguments[1] === 'function') {
		options = arguments[0];
		connectionListener = arguments[1];
	}
	options = _.chain(options)
			.pick('family', 'style')
			.defaults({
				family: 6,
				style: 'seqpacket',
				nativeSocket: null,
			})
			.value();
	var family = options.family,
		style = options.style;
	
	if (connectionListener !== undefined) {
		this.on('connection', connectionListener);
	}
	
	if (!usrsctp._initialized) usrsctp.init(9899);
	if (family !== 4 && family !== 6) {
		throw Error('Invalid IP family. Valid values are: 4, 6');
	}
	var typevalue;
	if (style === 'seqpacket') typevalue = false;
	else if (style === 'stream') typevalue = true;
	else throw Error('Invalid socket type. Valid values are: seqpacket, stream');
	var nativeSocket = this._nativeSocket = options.nativeSocket || syscall.socket(family, typevalue);
	this.family = family;
	this.style = style;
	
	if (style === 'stream') {
		nativeSocket.onNotification = function (notification) {
			console.log(notification);
			if (notification.new_sock !== undefined) {
				usrsctp._activeSockets++;
				usrsctp._syscall.ref();
				self.emit('connection', new Socket({
					nativeSocket: notification.new_sock,
					family: self.family,
					style: 'stream',
				}));
			} else {
				switch (notification) {
					case 'SCTP_ASSOC_CHANGE':
						//
						break;
					case 'SCTP_PEER_ADDR_CHANGE':
						break;
				}
			}
		};
		nativeSocket.onData = function (buf, info) {
			console.log(buf.length, info);
		};
	} else {
		nativeSocket.onNotification = function (notification) {
			console.log(notification);
		};
		nativeSocket.onData = function (buf, info) {
			console.log(buf.length, info);
		};
	}
};

Socket.prototype = {
	_nativeSocket: null,
	_associations: {},
	_style: null,
	_super: EventEmitter,
};

Socket.prototype.listen = function(options, callback) {
	if (typeof arguments[0] === 'number') {
		// port, host, callback
		var port = arguments[0],
			host = undefined,
			callback;
		var i = 1;
		if (Array.isArray(arguments[i])) {
			host = arguments[i];
			i++;
		} else if (typeof arguments[i] === 'string') {
			host.push(arguments[i]);
			i++;
		}
		if (typeof arguments[i] === 'function') {
			callback = arguments[i];
		}
		return this.listen({
			port: port,
			host: host
		}, callback);
	} else if (typeof options !== 'object') {
		throw Error('Invalid arguments');
	}
	options = _.chain(options)
			.pick('port', 'host')
			.defaults({
				host: [this.family == 6 ? '::' : '127.0.0.1']
			})
			.value();
	if (typeof options.host === 'string') {
		options.host = [options.host];
	}
	if (options.host.length < 1) throw Error('Invalid arguments');
	console.log(this, options);
	syscall.bind(this._nativeSocket, options.host[0]);
	for (var i = 1; i < options.host.length; i++) syscall.bindx(this._nativeSocket, options.host[i], true);
	syscall.listen(this._nativeSocket, true);
	
	usrsctp._activeSockets++;
	usrsctp._syscall.ref();
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

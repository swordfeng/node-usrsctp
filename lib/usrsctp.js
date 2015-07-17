var events = require('events'),
	EventEmitter = events.EventEmitter,
	net = require('net'),
	dns = require('dns'),
	domain = require('domain'),
	_ = require('underscore'),
	$ = require('async');
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

for (var name in syscall) {
	(function(name) {
		var callee = syscall[name];
		syscall[name] = 
			function () {
				console.log(name + '(' + Array.prototype.slice.call(arguments, 0).join() + ')');
				return callee.apply(null, arguments);
			};
	})(name);
}

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

function notificationListener(self) {
	return function (notification) {
		console.log(notification);
		
		if (self.style === 'stream') {
			switch (notification.type) {
			case 'SCTP_ASSOC_CHANGE': 
				switch (notification.state) {
				case 'SCTP_COMM_UP':
					if (notification.new_sock === undefined) throw Error('Unknown');
					usrsctp._activeSockets++;
					usrsctp._syscall.ref();
					self.emit('connection', new Socket({
						nativeSocket: notification.new_sock,
						family: self.family,
						style: 'stream',
					}));
					break;
				case 'SCTP_COMM_LOST':
					syscall.close(self._nativeSocket);
					self.emit('close', true);
					break;
				case 'SCTP_SHUTDOWN_COMP':
					syscall.close(self._nativeSocket);
					self.emit('close', false);
					break;
				case 'SCTP_CANT_STR_ASSOC':
					syscall.close(self._nativeSocket);
					self.emit('error', Error('Failed to start connection'));
					self.emit('close', true);
					break;
				}
				break;
			case 'SCTP_SHUTDOWN_EVENT':
				self.emit('end');
				break;
			case 'SCTP_SENDER_DRY_EVENT':
				self.emit('drain');
				break;
			} else {
			}
	};
}

function dataListener(self) {
	return function (buf, info) {
		console.log(buf.length, info);
	};
}

var Socket = usrsctp.Socket = function() {
	// if the function is not called as constructor,
	// then we recall it in a constructor style
	if (!(this instanceof Socket)) {
		return Socket.apply(Object.create(Socket.prototype), arguments);
	}
	
	// set up some default fields
	if (this._super) _.defaults(this, new this._super);
	this.domain = domain.create();
	
	var self = this;
	
	this.domain.on('error', function() {
		self.emit.apply(self, ['error'].concat(Array.prototype.slice(arguments, 0)));
	});
	
	// process arguments
	var options = {},
		connectionListener;
	if (typeof arguments[0] === 'function') {
		connectionListener = arguments[0];
	} else if (typeof arguments[0] === 'object') {
		options = arguments[0];
		connectionListener = arguments[1];
	}
	options = _.chain(options)
			.pick('family', 'style', 'nativeSocket')
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
	var nativeSocket = this._nativeSocket = options.nativeSocket === null ? syscall.socket(family, typevalue) : options.nativeSocket;
	this.family = family;
	this.style = style;
	
	nativeSocket.onNotification = notificationListener(this);
	nativeSocket.onData = dataListener(this);
};

Socket.prototype = {
	_nativeSocket: null,
	_associations: {},
	_style: null,
	_super: EventEmitter,
};

Socket.prototype.listen = function(options, callback) {
	var self = this;
	if (typeof arguments[0] === 'number') {
		// port, host, backlog, callback
		var port = arguments[0],
			host,
			backlog,
			callback;
		var i = 1;
		if (Array.isArray(arguments[i])) {
			host = arguments[i];
			i++;
		} else if (typeof arguments[i] === 'string') {
			host = [arguments[i]];
			i++;
		}
		if (typeof arguments[i] === 'number') {
			backlog = arguments[i++];
			if (!Number.isSafeInteger(backlog) || backlog < 0) {
				throw Error('Invalid argument: backlog');
			}
		}
		if (typeof arguments[i] === 'function') {
			callback = arguments[i];
		}
		return this.listen({
			port: port,
			host: host,
			backlog: backlog
		}, callback);
	} else if (typeof options !== 'object') {
		throw Error('Invalid arguments');
	}
	
	// options, callback
	options = _.chain(options)
			.pick('port', 'host', 'backlog')
			.defaults({
				host: [this.family == 6 ? '::' : '0.0.0.0'],
				backlog: 0xffffffff,
			})
			.value();
	
	if (!Number.isSafeInteger(options.port) || options.port < 0 || options.port > 65535) {
		throw Error('Invalid port number');
	}
	
	if (typeof options.host === 'string') {
		options.host = [options.host];
	}
	
	if (!_.every(options.host, net.isIP)) {
		$.map(options.host, function (host, cb) {
			if (net.isIP(host)) cb(null, host);
			else dns.resolve(host, cb);
		}, function (err, res) {
			if (err) throw err;
			options.host = _.flatten(res);
			self.domain.run(_.bind(self.listen, self, options, callback));
		});
		return;
	}
	
	options.host = _.reduce(options.host, function(hosts, host) {
		if (net.isIP(host) === self.family) return hosts.concat(host);
		else if (self.family === 6) {
			host = '::ffff:' + host;
			if (net.isIPv6(host)) return hosts.concat(host);
		}
		return hosts;
	}, []);
	
	if (options.host.length < 1) throw Error('Invalid host(s)');
	
	syscall.bind(this._nativeSocket, options.host[0], options.port);
	for (var i = 1; i < options.host.length; i++) syscall.bindx(this._nativeSocket, options.host[i], options.port, true);
	syscall.listen(this._nativeSocket, options.backlog);
	if (typeof callback === 'function') process.nextTick(callback);
	usrsctp._activeSockets++;
	usrsctp._syscall.ref();
	this.emit('listening');
};
/*
Socket.prototype.connect = function(options, connectionListener) {
	if (typeof arguments[0] === 'number') {
		// port, host, connectionListener
	} else if (typeof arguments[0] !== 'object') {
		throw Error('Invalid arguments');
	}
	// options, connectionListener
	options = _.chain(options)
			.pick('port', 'host')
			.defaults({
				host: 
};

var Association = usrsctp.Association = function() {};


usrsctp.createServer = function() {
	return Socket.apply(Object.create(Socket.prototype), arguments);
};
usrsctp.createConnection;
usrsctp.connect = usrsctp.createConnection;
*/
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
	syscall.unref();
});

usrsctp.listenSIGINT(true);
syscall.setRecvBufferSize(1048576);
syscall.setSendBufferSize(1048576);

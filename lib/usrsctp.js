var events = require('events'),
	EventEmitter = events.EventEmitter,
	net = require('net'),
	dns = require('dns'),
	domain = require('domain'),
	util = require('util'),
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
				console.log(name + '(' + Array.prototype.slice.call(arguments, 0).map(_.partial(util.inspect, _, 1)).join() + ')');
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
			}
		} else {
			console.log(util.inspect(self));
			var assoc = self._associations[notification.assoc_id];
			switch (notification.type) {
			case 'SCTP_ASSOC_CHANGE': 
				switch (notification.state) {
				case 'SCTP_COMM_UP':
					self.emit('connection', assoc);
					assoc.emit('connection');
					break;
				case 'SCTP_COMM_LOST':
					assoc.emit('close', true);
					assoc._close();
					break;
				case 'SCTP_SHUTDOWN_COMP':
					assoc.emit('close', false);
					assoc._close();
					break;
				case 'SCTP_CANT_STR_ASSOC':
					self.emit('error', Error('Failed to start association'));
					assoc.emit('error', Error('Failed to start association'));
					assoc.emit('close', true);
					assoc._close();
					break;
				}
				break;
			case 'SCTP_SHUTDOWN_EVENT':
				assoc.emit('end');
				break;
			case 'SCTP_SENDER_DRY_EVENT':
				assoc.emit('drain');
				break;
			}
		}
	};
}

function dataListener(self) {
	return function (buf, info) {
		console.log(buf.length, info);
	};
}

var Socket = usrsctp.Socket = function(options, connectionListener) {
	// if the function is not called as constructor,
	// then we recall it in a constructor style
	if (!(this instanceof Socket)) {
		return Socket.apply(Object.create(Socket.prototype), arguments);
	}
	
	// process arguments
	if (typeof arguments[0] === 'function') {
		options = {};
		connectionListener = arguments[0];
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
	if (family !== 4 && family !== 6) {
		throw Error('Invalid IP family. Valid values are: 4, 6');
	}
	var type;
	if (style === 'seqpacket') type = false;
	else if (style === 'stream') type = true;
	else throw Error('Invalid socket type. Valid values are: seqpacket, stream');
	if (connectionListener && typeof connectionListener !== 'function') {
		throw Error('Invalid connectionListener');
	}
	
	// set up some default fields
	if (this._super) _.defaults(this, new this._super);
	this.domain = domain.create();
	this._associations = {};
	
	var self = this;
	
	this.domain.on('error', function() {
		self.emit.apply(self, ['error'].concat(Array.prototype.slice(arguments, 0)));
	});
	
	if (connectionListener !== undefined) {
		this.on('connection', connectionListener);
	}
	
	if (!usrsctp._initialized) usrsctp.init(9899);
	
	var nativeSocket = this._nativeSocket = options.nativeSocket === null ? syscall.socket(family, type) : options.nativeSocket;
	this.family = family;
	this.style = style;
	
	nativeSocket.onNotification = notificationListener(this);
	nativeSocket.onData = dataListener(this);
	return this;
};

Socket.prototype = {
	_nativeSocket: null,
	_associations: null,
	_style: null,
	_super: EventEmitter,
};

Socket.prototype.listen = function(options, callback) {
	var self = this;
	if (typeof arguments[0] === 'number') {
		// port[, host][, backlog][, callback]
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
		options = {
			port: port,
			host: host,
			backlog: backlog
		};
	} else if (typeof options !== 'object') {
		throw Error('Invalid arguments');
	}
	
	// options[, callback]
	options = _.chain(options)
			.pick('port', 'host', 'backlog')
			.defaults({
				host: [this.family == 6 ? '::' : '0.0.0.0'],
				backlog: 0xffffffff, // will be set to SOMAXCONN as default
			})
			.value();
	
	if (!Number.isSafeInteger(options.port) || options.port < 0 || options.port > 65535) {
		throw Error('Invalid port number');
	}
	
	if (typeof options.host === 'string') {
		options.host = [options.host];
	}
	
	if (!_.every(options.host, net.isIP)) {
		// should I use Promise?
		$.map(options.host, function (host, cb) {
			if (net.isIP(host)) cb(null, host);
			else dns.resolve(host, cb);
		}, function (err, res) {
			if (err) throw err;
			options.host = _.flatten(res);
			self.domain.run(_.bind(self.listen, self, options, callback));
		});
		return this;
	}
	
	options.host = _.reduce(options.host, function(hosts, host) {
		if (net.isIP(host) === self.family) return hosts.concat(host);
		else if (self.family === 6) {
			host = '::ffff:' + host;
			if (net.isIPv6(host)) return hosts.concat(host);
		} else throw Error('Invalid address(es)');
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
	return this;
};

Socket.prototype.connect = function(options, connectionListener) {
	if (typeof arguments[0] === 'number') {
		// port[, encap_port][, host][, connectionListener]
		var port = arguments[0],
			encap_port,
			host,
			connectionListener,
			i = 1;
		if (typeof arguments[i] === 'number') encap_port = arguments[i++];
		if (typeof arguments[i] === 'string') host = arguments[i++];
		if (typeof arguments[i] === 'function') connectionListener = arguments[i++];
		options = {
			port: port,
			encap_port: encap_port,
			host: host
		};
	} else if (typeof arguments[0] !== 'object') {
		throw Error('Invalid arguments');
	}
	
	var self = this;
	
	// options[, connectionListener]
	options = _.chain(options)
			.pick('port', 'encap_port', 'host')
			.defaults({
				host: self.family === 6 ? '::1' : '127.0.0.1',
				encap_port: 9899
			})
			.value();
	if (options.port === undefined) throw Error('Invalid arguments: no port');
	if (!Number.isSafeInteger(options.port) || options.port > 65535 || options.port <= 0) {
		throw Error('Invalid port number');
	}
	if (connectionListener && typeof connectionListener !== 'function') {
		throw Error('Invalid connectionListener');
	}
	
	if (!net.isIP(options.host)) {
		// should I use Promise?
		dns.resolve(options.host, function (err, res) {
			if (err) throw err;
			options.host = res[0];
			self.domain.run(_.bind(self.connect, self, options, connectionListener));
		});
		return this;
	}
	
	this.on('connection', connectionListener);
	var assoc_id = syscall.connect(self._nativeSocket, options.host, options.port, options.encap_port);
	if (this.style === 'seqpacket') new Association(this, assoc_id);
	usrsctp._activeSockets++;
	usrsctp._syscall.ref();
	syscall.send(this._nativeSocket, {assoc_id:assoc_id, stream_id:2, ppid:1234, unordered:false}, new Buffer(131072), 0, 131072);
	return this;
};

var Association = usrsctp.Association = function(socket, assoc_id) {
	// call as constructor
	if (!this instanceof Association) {
		return Association.apply(Object.create(Association), arguments);
	}
	
	// check arguments
	if (!this.socket instanceof Socket) throw Error('Invalid socket');
	
	if (this._super) _.defaults(this, new this._super);
	
	this.socket = socket;
	this.assoc_id = assoc_id;
	
	socket._associations[assoc_id] = this;
	
	return this;
};

Association.prototype = {
	socket: null,
	assoc_id: null,
	_super: EventEmitter,
};
/*
usrsctp.createServer = function() {
	return Socket.apply(null, arguments);
};
usrsctp.createConnection = function() {
	var sock = Socket(6, 'stream');
	sock.bind...
	return sock.connect...
};
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
syscall.setRecvBufferSize(524288);
syscall.setSendBufferSize(524288);

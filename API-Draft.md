APIs may to be implemented
===
usrsctp.setUDPPort
usrsctp.createSocket
socket.bind
socket.listen
socket.connect -> assoc
socket.send
socket.end/close
socket.on
	connect
	data
	end/close
	timeout
	drain
	error
assoc.socket
assoc.send
assoc.end/close
assoc.peeloff
assoc.on
	data
	message
	


#### usrsctp native apis: ####
init
socket
sendv
bind
bindx
listen
accept
peeloff
connect
connectx
close
finish
shutdown

setsockopt
getsockopt
getpaddrs
freepaddrs
getladdrs
freeladdrs
conninput
set_non_blocking
get_non_blocking
register_address
deregister_address
set_ulpinfo
dumppacket
freedumpbuffer
sysctl_set_*
sysctl_get_*

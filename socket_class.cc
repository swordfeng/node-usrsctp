#include "socket_class.h"

namespace usrsctp {
	
	std::unordered_map<struct socket*, Socket*> Socket::socket_map;
	Socket *Socket::sock;
	uv_mutex_t Socket::recv_lock;
	uv_async_t Socket::recv_event;
	void *Socket::buf;
	size_t Socket::len;
	
	void Socket::recv_async_cb(uv_async_t *handle) {
		// todo
		uv_mutex_unlock(&recv_lock);
	}
	
	int Socket::receive_cb(struct socket *sd, union sctp_sockstore addr,
			void *data,	size_t datalen, struct sctp_rcvinfo rcv, int flags, 
			void *ulp_info) {
		
		uv_mutex_lock(&recv_lock);
		
		auto socket_item = socket_map.find(sd);
		// if (socket_item == std::unordered_map::end) ...
		sock = socket_item->second;
		
		free(buf);
		buf = data;
		len = datalen;
		
		// todo
		
		uv_async_send(&recv_event);
		return 1;
	}

	void Socket::Init() {
		sock = nullptr;
		buf = nullptr;
		len = 0;
		uv_async_init(uv_default_loop(), &recv_event, recv_async_cb);
		uv_mutex_init(&recv_lock);
	}
	
	Socket::Socket(int af, int type) {
		sd = usrsctp_socket(af, type, IPPROTO_SCTP, receive_cb, nullptr, 0, nullptr);
		// if (!sd) ...
		socket_map.insert(std::make_pair(sd, this));
		wrapper = new SocketWrapper(this);
	}
	
	Socket::~Socket() {
		// todo
	}
	
	SocketWrapper *Socket::GetWrapper() {
		return wrapper;
	}
	
}

#include "socket_class.h"
#include <iostream>

namespace usrsctp {
	
	std::unordered_map<struct socket*, Socket*> Socket::socket_map;
	Socket *Socket::sock;
	uv_mutex_t Socket::recv_lock;
	uv_async_t Socket::recv_event;
	void *Socket::recv_buf;
	size_t Socket::recv_len;
	
	void Socket::recv_async_cb(uv_async_t *handle) {
		std::cout << "async recv" << std::endl;
		sock->get_wrapper()->recv_cb(recv_buf, recv_len);
		recv_buf = nullptr;
		uv_mutex_unlock(&recv_lock);
	}
	
	int Socket::receive_cb(struct socket *sd, union sctp_sockstore addr,
			void *data,	size_t datalen, struct sctp_rcvinfo rcv, int flags, 
			void *ulp_info) {
		if (data) {
			if (flags & MSG_NOTIFICATION) {
				// todo: notification
				// sender_dry, send_failed, shutdown
				std::cout << "notif" << std::endl;
			} else {
				// data
				uv_mutex_lock(&recv_lock);
				auto socket_item = socket_map.find(sd);
				// if (socket_item == std::unordered_map::end) ...
				sock = socket_item->second;
				recv_buf = data;
				recv_len = datalen;
				// todo: other recv info
				uv_async_send(&recv_event);
			}
		} else {
			std::cout << "no data" << std::endl;
		}
		return 1;
	}

	void Socket::Init() {
		sock = nullptr;
		recv_buf = nullptr;
		recv_len = 0;
		uv_async_init(uv_default_loop(), &recv_event, recv_async_cb);
		uv_mutex_init(&recv_lock);
	}
	
	Socket::Socket(int af, int type) {
		this->af = af;
		this->type = type;
		sd = usrsctp_socket(af, type, IPPROTO_SCTP, receive_cb, nullptr, 0, nullptr);
		// if (!sd) ...
		socket_map.insert(std::make_pair(sd, this));
		wrapper = new SocketWrapper(this);
	}
	
	Socket::~Socket() {
		// todo
	}
	
	SocketWrapper *Socket::get_wrapper() {
		return wrapper;
	}
	
	struct socket *Socket::get_sd() {
		return sd;
	}
	
	int Socket::get_af() {
		return af;
	}
	
	int Socket::get_type() {
		return type;
	}
	
}

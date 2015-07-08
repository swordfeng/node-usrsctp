#include "socket_class.h"
#include <iostream>
#include <cstring>
#include <node.h>

namespace usrsctp {
	
	std::unordered_map<struct socket*, Socket*> Socket::socket_map;
	Socket *Socket::sock;
	uv_mutex_t Socket::recv_lock;
	uv_async_t Socket::recv_event;
	void *Socket::recv_buf;
	size_t Socket::recv_len;
	int Socket::recv_flags;
	struct sctp_rcvinfo Socket::recv_info;
	struct sockaddr_storage Socket::recv_addr;
	
	void Socket::recv_async_cb(uv_async_t *handle) {
		if (recv_flags & MSG_NOTIFICATION) {
			sock->get_wrapper()->notif_cb((union sctp_notification *)recv_buf);
		} else {
			sock->get_wrapper()->recv_cb(recv_buf, recv_len, recv_flags, &recv_info, &recv_addr);
		}
		free(recv_buf);
		recv_buf = nullptr;
		uv_mutex_unlock(&recv_lock);
	}
	
	int Socket::receive_cb(struct socket *sd, union sctp_sockstore addr,
			void *data,	size_t datalen, struct sctp_rcvinfo rcv, int flags, 
			void *ulp_info) {
		if (data) {
			uv_mutex_lock(&recv_lock);
			auto socket_item = socket_map.find(sd);
			// if (socket_item == std::unordered_map::end) ...
			sock = socket_item->second;
			recv_buf = data;
			recv_len = datalen;
			recv_flags = flags;
			memcpy(&recv_info, &rcv, sizeof(struct sctp_rcvinfo));
			socklen_t addrlen = 0;
			switch (addr.sa.sa_family) {
				case AF_INET:
					addrlen = sizeof(struct sockaddr_in);
					break;
				case AF_INET6:
					addrlen = sizeof(struct sockaddr_in6);
					break;
			}
			memcpy(&recv_addr, &addr, addrlen);
			uv_async_send(&recv_event);
		} else {
			// ??
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
	
	void Socket::Exit() {
		std::cout << "clean up" << std::endl;
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

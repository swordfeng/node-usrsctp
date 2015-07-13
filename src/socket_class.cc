#include "usrsctp.h"
#include "socket_class.h"
#include <iostream>
#include <cstring>
#include <node.h>
#include <iomanip>
#include <cassert>

namespace usrsctp {
	
	std::unordered_set<Socket *> Socket::socket_list;
	Socket *Socket::sock;
	uv_mutex_t Socket::recv_lock;
	uv_async_t Socket::recv_event;
	void *Socket::recv_buf;
	size_t Socket::recv_len;
	int Socket::recv_flags;
	struct sctp_rcvinfo *Socket::recv_info;
	struct sockaddr_storage *Socket::recv_addr;
	
	void Socket::recv_async_cb(uv_async_t *handle) {
		Socket *sock = Socket::sock;
		void *recv_buf = Socket::recv_buf;
		size_t recv_len = Socket::recv_len;
		int recv_flags = Socket::recv_flags;
		struct sctp_rcvinfo *recv_info = Socket::recv_info;
		struct sockaddr_storage *recv_addr = Socket::recv_addr;
		Socket::recv_buf = nullptr;
		Socket::recv_info = nullptr;
		Socket::recv_addr = nullptr;
		uv_mutex_unlock(&recv_lock);
		
		if (recv_flags & MSG_NOTIFICATION) {
			sock->get_wrapper()->notif_cb((union sctp_notification *)recv_buf);
		} else {
			sock->get_wrapper()->recv_cb(recv_buf, recv_len, recv_flags, recv_info, recv_addr);
		}
		free(recv_buf);
		delete recv_info;
		delete recv_addr;
	}
	
	int Socket::receive_cb(struct socket *sd, union sctp_sockstore addr,
			void *data,	size_t datalen, struct sctp_rcvinfo rcv, int flags, 
			void *ulp_info) {
		Socket *sockptr = (Socket *)ulp_info;
		if (data) {
			assert(socket_list.find(sockptr) != socket_list.end());
			uv_mutex_lock(&recv_lock);
			delete recv_info;
			delete recv_addr;
			recv_info = new struct sctp_rcvinfo;
			recv_addr = new struct sockaddr_storage;
			sock = sockptr;
			recv_buf = data;
			recv_len = datalen;
			recv_flags = flags;
			memcpy(recv_info, &rcv, sizeof(struct sctp_rcvinfo));
			socklen_t addrlen = 0;
			switch (addr.sa.sa_family) {
				case AF_INET:
					addrlen = sizeof(struct sockaddr_in);
					break;
				case AF_INET6:
					addrlen = sizeof(struct sockaddr_in6);
					break;
			}
			memcpy(recv_addr, &addr, addrlen);
			uv_async_send(&recv_event);
		} else {
			// shutdown - will have a notification
		}
		return 1;
	}

	void Socket::Init() {
		sock = nullptr;
		recv_buf = nullptr;
		recv_info = nullptr;
		recv_addr = nullptr;
		recv_len = 0;
		uv_async_init(uv_default_loop(), &recv_event, recv_async_cb);
		uv_mutex_init(&recv_lock);
		uv_unref((uv_handle_t *)&recv_event);
	}
	
	void Socket::End(bool force) {
		uv_mutex_lock(&recv_lock);
		while (socket_list.begin() != socket_list.end()) {
			auto sock = socket_list.begin();
			if (force) {
				struct sctp_sndinfo snd_info;
				memset(&snd_info, 0, sizeof(struct sctp_sndinfo));
				snd_info.snd_flags = SCTP_ABORT|SCTP_SENDALL;
				snd_info.snd_assoc_id = SCTP_CURRENT_ASSOC;
				usrsctp_sendv(**sock, nullptr, 0, nullptr, 0, &snd_info, sizeof(snd_info), SCTP_SENDV_SNDINFO, 0);
			}
			usrsctp_close(**sock);
			delete *sock;
		}
		usrsctp_finish();
		delete recv_info;
		delete recv_addr;
		if (recv_buf) free(recv_buf);
		uv_close((uv_handle_t *)&recv_event, [](uv_handle_t *){});
		uv_mutex_unlock(&recv_lock);
		uv_mutex_destroy(&recv_lock);
	}
	
	void Socket::ref() {
		uv_ref((uv_handle_t *)&recv_event);
	}
	
	void Socket::unref() {
		uv_unref((uv_handle_t *)&recv_event);
	}
	
	Socket::Socket(int af, int type) {
		this->af = af;
		this->type = type;
		assert(sd = usrsctp_socket(af, type, IPPROTO_SCTP, receive_cb, nullptr, 0, this));
		socket_list.insert(this);
		wrapper = new SocketWrapper(this);
		if (type == SOCK_STREAM) {
			int on = 1;
			usrsctp_setsockopt(sd, IPPROTO_SCTP, SCTP_REUSE_PORT, &on, sizeof(int));
		}
	}
	
	Socket::~Socket() {
		wrapper->SetInvalid();
		socket_list.erase(this);
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

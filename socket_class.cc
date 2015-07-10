#include "socket_class.h"
#include <iostream>
#include <cstring>
#include <node.h>
#include <iomanip>

namespace usrsctp {
	
	std::unordered_map<struct socket*, Socket*> Socket::socket_map;
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
		if (data) {
			uv_mutex_lock(&recv_lock);
			delete recv_info;
			delete recv_addr;
			recv_info = new struct sctp_rcvinfo;
			recv_addr = new struct sockaddr_storage;
			auto socket_item = socket_map.find(sd);
			// if (socket_item == std::unordered_map::end) ...
			sock = socket_item->second;
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
	
	static void empty_callback(uv_handle_t *) {}
	void Socket::End(bool force) {
		while (socket_map.size() > 0) {
			auto mapitem = socket_map.begin();
			if (force) {
				struct sctp_sndinfo snd_info;
				memset(&snd_info, 0, sizeof(struct sctp_sndinfo));
				snd_info.snd_flags = SCTP_ABORT | SCTP_SENDALL;
				snd_info.snd_assoc_id = SCTP_CURRENT_ASSOC;
				usrsctp_sendv(*sock, nullptr, 0, nullptr, 0, &snd_info, sizeof(snd_info), SCTP_SENDV_SNDINFO, 0);
			}
			usrsctp_close(mapitem->first);
			delete mapitem->second;
		}
		usrsctp_finish();
		delete recv_info;
		delete recv_addr;
		if (recv_buf) free(recv_buf);
		uv_close((uv_handle_t *)&recv_event, empty_callback);
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
		sd = usrsctp_socket(af, type, IPPROTO_SCTP, receive_cb, nullptr, 0, nullptr);
		// if (!sd) ...
		socket_map.insert(std::make_pair(sd, this));
		wrapper = new SocketWrapper(this);
	}
	
	Socket::~Socket() {
		// todo: check and close socket
		wrapper->SetInvalid();
		socket_map.erase(sd);
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

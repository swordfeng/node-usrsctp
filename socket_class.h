#ifndef NODE_SCTP_SOCKET_CTX_H
#define NODE_SCTP_SOCKET_CTX_H

#include <unordered_map>
#include <uv.h>
#include "socket_wrapper_class.h"

namespace usrsctp {
	
	class SocketWrapper;
	
	class Socket {
		public:
			static void Init();
		private:
			static void recv_async_cb(uv_async_t *handle);
			static int receive_cb(struct socket *sd, union sctp_sockstore addr,
				void *data,	size_t datalen, struct sctp_rcvinfo rcv, int flags, 
				void *ulp_info);
			
			static std::unordered_map<struct socket*, Socket*> socket_map;
			static Socket *sock;
			static uv_mutex_t recv_lock;
			static uv_async_t recv_event;
			static void *recv_buf;
			static size_t recv_len;
		public:
			Socket(int af, int type);
			SocketWrapper *get_wrapper();
			~Socket();
			ssize_t send(const void *buf, size_t len);
		private:
			struct socket *sd;
			SocketWrapper *wrapper;
	};
}

#endif

#ifndef NODE_SCTP_SOCKET_CTX_H
#define NODE_SCTP_SOCKET_CTX_H

#include <unordered_map>
#include <uv.h>
#include <usrsctp.h>
#include "socket_wrapper_class.h"

namespace usrsctp {
	
	class SocketWrapper;
	
	class Socket {
		public:
			static void Init();
			static void End();
			static void ref();
			static void unref();
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
			static int recv_flags; 
			static struct sctp_rcvinfo *recv_info;
			static struct sockaddr_storage *recv_addr;
		public:
			Socket(int af, int type);
			SocketWrapper *get_wrapper();
			struct socket *get_sd();
			int get_af();
			int get_type();
			inline operator struct socket *() {
				return sd;
			}
			~Socket();
		private:
			struct socket *sd;
			SocketWrapper *wrapper;
			int af;
			int type;
	};
}

#endif

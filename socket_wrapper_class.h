#ifndef NODE_SCTP_SOCKET_WRAPPER_H
#define NODE_SCTP_SOCKET_WRAPPER_H

#include <node.h>
#include <node_object_wrap.h>
#include <usrsctp.h>
#include "socket_class.h"

namespace usrsctp {
	using namespace v8;
	using namespace node;
	
	class Socket;
	
	class SocketWrapper : public ObjectWrap {
		public:
			static void Init(Local<Object> exports);
			static SocketWrapper *FromObject(Local<Value> obj);
		private:
			static Persistent<FunctionTemplate> constructor_tpl;
		public:
			SocketWrapper(Socket *sock);
			Local<Object> ToObject();
			Socket *GetSocket();
			void SetInvalid();
			void recv_cb(void *buf, size_t len, int flags, struct sctp_rcvinfo *info, struct sockaddr_storage *addr);
			void notif_cb(union sctp_notification *notification);
		private:
			~SocketWrapper();
			Socket *sock;
    };
}

#endif

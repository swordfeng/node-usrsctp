#include <string>
#include <cstring>
#include <node.h>
#include <node_buffer.h>
#include <usrsctp.h>
#include "socket_class.h"

namespace usrsctp {
	using namespace v8;
	
	//usrsctp_init
	void init(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		uint32_t udp_port = args[0]->Uint32Value();
		if (udp_port > 65536) {
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "Invalid port number")));
			return;
		}
		usrsctp_init(static_cast<uint16_t>(udp_port), nullptr, nullptr);
		args.GetReturnValue().Set(Undefined(isolate));
	}

	//usrsctp_socket
	//.socket(4|6, true(seqpacket)|false(stream))
	void socket(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		
		int domain = args[0]->Uint32Value() == 4 ? AF_INET : AF_INET6;
		
		/*
		std::string type_str = std::string(*String::Utf8Value(args[1]->ToString()));
		int type = type_str == "stream" ? SOCK_STREAM : SOCK_SEQPACKET;
		*/
		// now we only support creating one-to-many socket
		int type = SOCK_SEQPACKET;
		
		Socket *sock = new Socket(domain, type);
		
		const int on = 1;
		if (usrsctp_setsockopt(*sock, IPPROTO_SCTP, SCTP_I_WANT_MAPPED_V4_ADDR, (const void*)&on, (socklen_t)sizeof(int)) < 0) {
			// ??
		}
		if (usrsctp_setsockopt(*sock, IPPROTO_SCTP, SCTP_RECVRCVINFO, &on, sizeof(int)) < 0) {
			// ??
		}
		
		Local<Object> obj = sock->get_wrapper()->ToObject();
		/*
		Local<Function> cb = Local<Function>::Cast(args[2]);
		obj->Set(String::NewFromUtf8(isolate, "callback"), cb);
		*/
		
		args.GetReturnValue().Set(obj);
	}

	//usrsctp_sendv
	//.send(socket, {assoc_id:0, stream_id:0, ppid:0, context(use with notif), flags}, buffer, start, length)
	void send(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		
		SocketWrapper *wrapper = SocketWrapper::FromObject(args[0]->ToObject());
		if (!wrapper) return;
		Socket *sock = wrapper->GetSocket();
		if (!sock) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Socket is no longer valid")));
			return;
		}
		
		Local<Object> options = args[1]->ToObject();
		Local<Object> buffer = args[2]->ToObject();
		unsigned int start = args[3]->Uint32Value();
		unsigned int len = args[4]->Uint32Value();
		
		// make snd_info
		struct sctp_sndinfo snd_info;
		snd_info.snd_sid = 0; // stream_id
		snd_info.snd_flags = 0;
		snd_info.snd_ppid = 0; // ppid
		snd_info.snd_context = 0; // not used now
		snd_info.snd_assoc_id = 0; // assoc_id
		
		usrsctp_sendv(*sock, Buffer::Data(buffer) + start, len, nullptr, 0, &snd_info, sizeof(snd_info), SCTP_SENDV_SNDINFO, 0);
	}
	
	//usrsctp_bindx
	//.bind(socket, addr, port, flag = true (add), false (remove))
	void bind(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		
		SocketWrapper *wrapper = SocketWrapper::FromObject(args[0]->ToObject());
		if (!wrapper) return;
		Socket *sock = wrapper->GetSocket();
		if (!sock) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Socket is no longer valid")));
			return;
		}
		
		struct sockaddr_storage saddr;
		memset(&saddr, 0, sizeof(struct sockaddr_storage));
		char *addrstr = *String::Utf8Value(args[1]->ToString());
		in_port_t port = htons(static_cast<uint16_t>(args[2]->Uint32Value()));
		
		if (sock->get_af() == AF_INET) {
			// ipv4
			struct sockaddr_in *saddr4 = (struct sockaddr_in *)(&saddr);
			saddr4->sin_family = AF_INET;
			saddr4->sin_port = port;
			int err = inet_pton(AF_INET, addrstr, &saddr4->sin_addr);
		} else if (sock->get_af() == AF_INET6) {
			// ipv6
			struct sockaddr_in6 *saddr6 = (struct sockaddr_in6 *)(&saddr);
			saddr6->sin6_family = AF_INET6;
			saddr6->sin6_port = port;
			int err = inet_pton(AF_INET, addrstr, &saddr6->sin6_addr);
		} else {
			// error
		}
		
		bool flag = args[3]->ToBoolean()->Value();
		
		usrsctp_bindx(*sock, (struct sockaddr *)&saddr, 1, flag ? SCTP_BINDX_ADD_ADDR : SCTP_BINDX_REM_ADDR);
	}
	
	//usrsctp_listen
	//.listen(socket, flag = true (on), false (off))
	//create accept thread when needed
	
	//usrsctp_peeloff
	//.peeloff(socket, assoc_id)
	
	//usrsctp_connectx
	//.connect(socket, addrs)

	void Init(Handle<Object> exports) {
		Socket::Init();
		SocketWrapper::Init(exports);
		NODE_SET_METHOD(exports, "init", init);
		NODE_SET_METHOD(exports, "socket", socket);
		NODE_SET_METHOD(exports, "send", send);
	}

	NODE_MODULE(usrsctp, Init)
	
}

#include "usrsctp.h"
#include <string>
#include <cstring>
#include <node.h>
#include <node_buffer.h>
#include <usrsctp.h>
#include "socket_class.h"
#include <iostream>
#include <cassert>
#include <cerrno>
#include <cstdio>
using std::cout;
using std::endl;
using std::cerr;

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
		usrsctp_init(static_cast<uint16_t>(udp_port), nullptr, (void (*)(const char*, ...))printf);
	}

	//usrsctp_socket
	//.socket(4|6, true(seqpacket)|false(stream))
	void socket(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		
		int domain = args[0]->Uint32Value() == 4 ? AF_INET : AF_INET6;
		
		/*
		int type = type_str == "stream" ? SOCK_STREAM : SOCK_SEQPACKET;
		*/
		// now we only support creating one-to-many socket
		int type = SOCK_SEQPACKET;
		
		Socket *sock = new Socket(domain, type);
		
		const int on = 1;
		if (domain == AF_INET6) {
			assert(usrsctp_setsockopt(*sock, IPPROTO_SCTP, SCTP_I_WANT_MAPPED_V4_ADDR, (const void*)&on, (socklen_t)sizeof(int)) >= 0);
		}
		assert(usrsctp_setsockopt(*sock, IPPROTO_SCTP, SCTP_RECVRCVINFO, &on, sizeof(int)) >= 0);
		
		struct sctp_event event;
		uint16_t event_types[] = {SCTP_ASSOC_CHANGE,
						  SCTP_PEER_ADDR_CHANGE,
						  SCTP_REMOTE_ERROR,
						  SCTP_SHUTDOWN_EVENT,
						  SCTP_ADAPTATION_INDICATION,
						  SCTP_PARTIAL_DELIVERY_EVENT,
						  SCTP_AUTHENTICATION_EVENT,
						  SCTP_SENDER_DRY_EVENT,
						  SCTP_SEND_FAILED_EVENT};
		memset(&event, 0, sizeof(event));
		event.se_assoc_id = SCTP_FUTURE_ASSOC;
		event.se_on = 1;
		for (size_t i = 0; i < sizeof(event_types) / sizeof(uint16_t); i++) {
			event.se_type = event_types[i];
			assert(usrsctp_setsockopt(*sock, IPPROTO_SCTP, SCTP_EVENT, &event, sizeof(struct sctp_event)) >= 0);
		}
		
		Local<Object> obj = sock->get_wrapper()->ToObject();
		args.GetReturnValue().Set(obj);
	}

	//usrsctp_sendv
	//.send(socket, {assoc_id:0, stream_id:0, ppid:0, unordered:false}, buffer, start, length)
	void send(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		
		SocketWrapper *wrapper = SocketWrapper::FromObject(args[0]->ToObject());
		if (!wrapper) return;
		Socket *sock = wrapper->GetSocket();
		if (!sock) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid socket")));
			return;
		}
		
		Local<Object> options = args[1]->ToObject();
		Local<Object> buffer = args[2]->ToObject();
		unsigned int start = args[3]->Uint32Value();
		unsigned int len = args[4]->Uint32Value();
		
		// make snd_info
		struct sctp_sndinfo snd_info;
		snd_info.snd_sid = options->Get(String::NewFromUtf8(isolate, "stream_id"))->Uint32Value(); // stream_id
		snd_info.snd_flags = 0;
		if (options->Get(String::NewFromUtf8(isolate, "unordered"))->ToBoolean()->Value()) snd_info.snd_flags |= SCTP_UNORDERED;
		// todo: sendall, addr_over
		snd_info.snd_ppid = htonl(options->Get(String::NewFromUtf8(isolate, "ppid"))->Uint32Value()); // ppid
		snd_info.snd_context = 0; // not used now
		snd_info.snd_assoc_id = options->Get(String::NewFromUtf8(isolate, "assoc_id"))->Uint32Value(); // assoc_id
		
		ssize_t sendlen = usrsctp_sendv(*sock, Buffer::Data(buffer) + start, len, nullptr, 0, &snd_info, sizeof(snd_info), SCTP_SENDV_SNDINFO, 0);
		if (sendlen < 0) {
			Local<Object> err = Local<Object>::Cast(Exception::Error(String::NewFromUtf8(isolate, strerror(errno))));
#define SETERRCODE(x) if (errno == x) err->Set(String::NewFromUtf8(isolate, "code"), String::NewFromUtf8(isolate, #x))
			SETERRCODE(EAGAIN);
			SETERRCODE(EMSGSIZE);
#undef SETERRORCODE
			isolate->ThrowException(err);
			return;
		}
	}
	
	//usrsctp_bind
	//.bind(socket, addr, port)
	void bind(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		
		SocketWrapper *wrapper = SocketWrapper::FromObject(args[0]->ToObject());
		if (!wrapper) return;
		Socket *sock = wrapper->GetSocket();
		if (!sock) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid socket")));
			return;
		}
		
		struct sockaddr_storage saddr;
		memset(&saddr, 0, sizeof(struct sockaddr_storage));
		std::string addrstr = *String::Utf8Value(args[1]->ToString());
		in_port_t port = htons(static_cast<uint16_t>(args[2]->Uint32Value()));
		socklen_t slen;
		
		int err;
		if (sock->get_af() == AF_INET) {
			// ipv4
			struct sockaddr_in *saddr4 = (struct sockaddr_in *)(&saddr);
			saddr4->sin_family = AF_INET;
			saddr4->sin_port = port;
			err = uv_inet_pton(AF_INET, addrstr.c_str(), &saddr4->sin_addr);
			slen = sizeof(struct sockaddr_in);
		} else if (sock->get_af() == AF_INET6) {
			// ipv6
			struct sockaddr_in6 *saddr6 = (struct sockaddr_in6 *)(&saddr);
			saddr6->sin6_family = AF_INET6;
			saddr6->sin6_port = port;
			err = uv_inet_pton(AF_INET6, addrstr.c_str(), &saddr6->sin6_addr);
			slen = sizeof(struct sockaddr_in6);
		} else {
			// error
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Unexpected Error")));
			return;
		}
		
		if (err) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid address")));
			return;
		}
		
		if (usrsctp_bind(*sock, (struct sockaddr *)&saddr, slen) < 0) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, strerror(errno))));
			return;
		}
	}
	
	//usrsctp_bindx
	//.bindx(socket, addr, port, enabled)
	void bindx(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		
		SocketWrapper *wrapper = SocketWrapper::FromObject(args[0]->ToObject());
		if (!wrapper) return;
		Socket *sock = wrapper->GetSocket();
		if (!sock) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid socket")));
			return;
		}
		
		struct sockaddr_storage saddr;
		memset(&saddr, 0, sizeof(struct sockaddr_storage));
		std::string addrstr = *String::Utf8Value(args[1]->ToString());
		in_port_t port = htons(static_cast<uint16_t>(args[2]->Uint32Value()));
		
		int err;
		if (sock->get_af() == AF_INET) {
			// ipv4
			struct sockaddr_in *saddr4 = (struct sockaddr_in *)(&saddr);
			saddr4->sin_family = AF_INET;
			saddr4->sin_port = port;
			err = uv_inet_pton(AF_INET, addrstr.c_str(), &saddr4->sin_addr);
		} else if (sock->get_af() == AF_INET6) {
			// ipv6
			struct sockaddr_in6 *saddr6 = (struct sockaddr_in6 *)(&saddr);
			saddr6->sin6_family = AF_INET6;
			saddr6->sin6_port = port;
			err = uv_inet_pton(AF_INET6, addrstr.c_str(), &saddr6->sin6_addr);
		} else {
			// error
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Unexpected Error")));
			return;
		}
		
		if (err) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid address")));
			return;
		}
		
		bool flag = args[3]->ToBoolean()->Value();
		
		if (usrsctp_bindx(*sock, (struct sockaddr *)&saddr, 1, flag ? SCTP_BINDX_ADD_ADDR : SCTP_BINDX_REM_ADDR) < 0) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, strerror(errno))));
			return;
		}
	}
	
	//usrsctp_listen
	//.listen(socket, flag = true (on), false (off))
	//create accept thread when needed
	void listen(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		
		SocketWrapper *wrapper = SocketWrapper::FromObject(args[0]->ToObject());
		if (!wrapper) return;
		Socket *sock = wrapper->GetSocket();
		if (!sock) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid socket")));
			return;
		}
		
		bool flag = args[1]->ToBoolean()->Value();
		
		if (usrsctp_listen(*sock, flag ? 1 : 0) < 0) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, strerror(errno))));
			return;
		}
		
		if (sock->get_type() == SOCK_STREAM) {
			// todo: needs accept
		}
	}
	
	//usrsctp_peeloff
	//.peeloff(socket, assoc_id)
	
	//usrsctp_connectx
	//.connect(socket, addr, port, encap_port)
	void connect(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		
		SocketWrapper *wrapper = SocketWrapper::FromObject(args[0]->ToObject());
		if (!wrapper) return;
		Socket *sock = wrapper->GetSocket();
		if (!sock) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid socket")));
			return;
		}
		
		// todo: support multiple addresses
		struct sockaddr_storage saddr;
		memset(&saddr, 0, sizeof(struct sockaddr_storage));
		std::string addrstr = *String::Utf8Value(args[1]->ToString());
		in_port_t port = htons(static_cast<uint16_t>(args[2]->Uint32Value()));
		in_port_t encap_port = htons(static_cast<uint16_t>(args[3]->Uint32Value()));
		int err;
		
		if (sock->get_af() == AF_INET) {
			// ipv4
			struct sockaddr_in *saddr4 = (struct sockaddr_in *)(&saddr);
			saddr4->sin_family = AF_INET;
			saddr4->sin_port = port;
			err = uv_inet_pton(AF_INET, addrstr.c_str(), &saddr4->sin_addr);
		} else if (sock->get_af() == AF_INET6) {
			// ipv6
			struct sockaddr_in6 *saddr6 = (struct sockaddr_in6 *)(&saddr);
			saddr6->sin6_family = AF_INET6;
			saddr6->sin6_port = port;
			err = uv_inet_pton(AF_INET6, addrstr.c_str(), &saddr6->sin6_addr);
		} else {
			// error
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Unexpected Error")));
			return;
		}
		
		if (err) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid address")));
			return;
		}
		
		struct sctp_udpencaps encaps;
		memcpy(&encaps.sue_address, &saddr, sizeof(struct sockaddr_storage));
		encaps.sue_assoc_id = SCTP_FUTURE_ASSOC;
		encaps.sue_port = encap_port;
		if (usrsctp_setsockopt(*sock, IPPROTO_SCTP, SCTP_REMOTE_UDP_ENCAPS_PORT, (const void*)&encaps, (socklen_t)sizeof(struct sctp_udpencaps)) < 0) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Fail to set Remote Encapsulation Port")));
			return;
		}
		
		sctp_assoc_t assoc_id;
		if (usrsctp_connectx(*sock, (struct sockaddr *)&saddr, 1, &assoc_id) < 0) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, strerror(errno))));
			return;
		}
		
		args.GetReturnValue().Set(Number::New(isolate, assoc_id));
	}
	
	void end(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		
		SocketWrapper *wrapper = SocketWrapper::FromObject(args[0]->ToObject());
		if (!wrapper) return;
		Socket *sock = wrapper->GetSocket();
		if (!sock) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid socket")));
			return;
		}
		sctp_assoc_t assoc_id = static_cast<sctp_assoc_t>(args[1]->Uint32Value());
		// make snd_info
		struct sctp_sndinfo snd_info;
		memset(&snd_info, 0, sizeof(struct sctp_sndinfo));
		snd_info.snd_flags = SCTP_EOF;
		snd_info.snd_assoc_id = assoc_id;

		usrsctp_sendv(*sock, nullptr, 0, nullptr, 0, &snd_info, sizeof(snd_info), SCTP_SENDV_SNDINFO, 0);
	}

	void abort(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		
		SocketWrapper *wrapper = SocketWrapper::FromObject(args[0]->ToObject());
		if (!wrapper) return;
		Socket *sock = wrapper->GetSocket();
		if (!sock) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid socket")));
			return;
		}
		sctp_assoc_t assoc_id = static_cast<sctp_assoc_t>(args[1]->Uint32Value());
		// make snd_info
		struct sctp_sndinfo snd_info;
		memset(&snd_info, 0, sizeof(struct sctp_sndinfo));
		snd_info.snd_flags = SCTP_ABORT;
		snd_info.snd_assoc_id = assoc_id;

		usrsctp_sendv(*sock, nullptr, 0, nullptr, 0, &snd_info, sizeof(snd_info), SCTP_SENDV_SNDINFO, 0);
	}
		
	void close(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		
		SocketWrapper *wrapper = SocketWrapper::FromObject(args[0]->ToObject());
		if (!wrapper) return;
		Socket *sock = wrapper->GetSocket();
		if (!sock) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid socket")));
			return;
		}
		usrsctp_close(*sock);
		delete sock;
	}
	
	void get_local_addr(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		
		SocketWrapper *wrapper = SocketWrapper::FromObject(args[0]->ToObject());
		if (!wrapper) return;
		Socket *sock = wrapper->GetSocket();
		if (!sock) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid socket")));
			return;
		}
		sctp_assoc_t assoc_id = static_cast<sctp_assoc_t>(args[1]->Uint32Value());
		struct sockaddr *raddrs, *raddr;
		int addrcnt = usrsctp_getladdrs(*sock, assoc_id, &raddrs);
		if (addrcnt < 0) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "getladdrs")));
			usrsctp_freeladdrs(raddrs);
			return;
		}
		Local<Array> addrarr = Array::New(isolate, addrcnt);
		char addrstr[50];
		
		raddr = raddrs;
		for (int i = 0; i < addrcnt; i++) {
			if (raddr->sa_family == AF_INET) {
				auto addr = (struct sockaddr_in *)raddr;
				if (uv_inet_ntop(AF_INET, &addr->sin_addr, addrstr, 50)) {
					isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, strerror(errno))));
					break;
				}
				Local<Object> addrobj = Object::New(isolate);
				addrobj->Set(String::NewFromUtf8(isolate, "address"), String::NewFromUtf8(isolate, addrstr));
				addrobj->Set(String::NewFromUtf8(isolate, "port"), Number::New(isolate, ntohs(addr->sin_port)));
				addrarr->Set(i, addrobj);
				raddr = (struct sockaddr *)(addr + 1);
			} else if (raddr->sa_family == AF_INET6) {
				auto addr6 = (struct sockaddr_in6 *)raddr;
				if (uv_inet_ntop(AF_INET6, &addr6->sin6_addr, addrstr, 50)) {
					isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, strerror(errno))));
					break;
				}
				Local<Object> addrobj = Object::New(isolate);
				addrobj->Set(String::NewFromUtf8(isolate, "address"), String::NewFromUtf8(isolate, addrstr));
				addrobj->Set(String::NewFromUtf8(isolate, "port"), Number::New(isolate, ntohs(addr6->sin6_port)));
				addrarr->Set(i, addrobj);
				raddr = (struct sockaddr *)(addr6 + 1);
			} else {
				isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Unknown address family")));
			}
		}
		args.GetReturnValue().Set(addrarr);
		usrsctp_freeladdrs(raddrs);
	}
	
	void get_remote_addr(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		
		SocketWrapper *wrapper = SocketWrapper::FromObject(args[0]->ToObject());
		if (!wrapper) return;
		Socket *sock = wrapper->GetSocket();
		if (!sock) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid socket")));
			return;
		}
		sctp_assoc_t assoc_id = static_cast<sctp_assoc_t>(args[1]->Uint32Value());
		struct sockaddr *raddrs, *raddr;
		int addrcnt = usrsctp_getpaddrs(*sock, assoc_id, &raddrs);
		if (addrcnt < 0) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "getladdrs")));
			usrsctp_freeladdrs(raddrs);
			return;
		}
		Local<Array> addrarr = Array::New(isolate, addrcnt);
		char addrstr[50];
		
		raddr = raddrs;
		for (int i = 0; i < addrcnt; i++) {
			if (raddr->sa_family == AF_INET) {
				auto addr = (struct sockaddr_in *)raddr;
				if (uv_inet_ntop(AF_INET, &addr->sin_addr, addrstr, 50)) {
					isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, strerror(errno))));
					break;
				}
				Local<Object> addrobj = Object::New(isolate);
				addrobj->Set(String::NewFromUtf8(isolate, "address"), String::NewFromUtf8(isolate, addrstr));
				addrobj->Set(String::NewFromUtf8(isolate, "port"), Number::New(isolate, ntohs(addr->sin_port)));

				struct sctp_udpencaps encaps;
				memcpy(&encaps.sue_address, raddr, sizeof(struct sockaddr_in));
				encaps.sue_assoc_id = assoc_id;
				encaps.sue_port = 0;
				socklen_t encaps_len = (socklen_t)sizeof(struct sctp_udpencaps);
				if (usrsctp_getsockopt(*sock, IPPROTO_SCTP, SCTP_REMOTE_UDP_ENCAPS_PORT, &encaps, &encaps_len) < 0) {
					isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Fail to get Remote Encapsulation Port")));
				}
				addrobj->Set(String::NewFromUtf8(isolate, "encap_port"), Number::New(isolate, ntohs(encaps.sue_port)));

				addrarr->Set(i, addrobj);
				raddr = (struct sockaddr *)(addr + 1);
			} else if (raddr->sa_family == AF_INET6) {
				auto addr6 = (struct sockaddr_in6 *)raddr;
				if (uv_inet_ntop(AF_INET6, &addr6->sin6_addr, addrstr, 50)) {
					isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, strerror(errno))));
					break;
				}
				
				Local<Object> addrobj = Object::New(isolate);
				addrobj->Set(String::NewFromUtf8(isolate, "address"), String::NewFromUtf8(isolate, addrstr));
				addrobj->Set(String::NewFromUtf8(isolate, "port"), Number::New(isolate, ntohs(addr6->sin6_port)));
				
				struct sctp_udpencaps encaps;
				memcpy(&encaps.sue_address, raddr, sizeof(struct sockaddr_in6));
				encaps.sue_assoc_id = assoc_id;
				encaps.sue_port = 0;
				socklen_t encaps_len = (socklen_t)sizeof(struct sctp_udpencaps);
				if (usrsctp_getsockopt(*sock, IPPROTO_SCTP, SCTP_REMOTE_UDP_ENCAPS_PORT, &encaps, &encaps_len) < 0) {
					isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Fail to get Remote Encapsulation Port")));
				}
				addrobj->Set(String::NewFromUtf8(isolate, "encap_port"), Number::New(isolate, ntohs(encaps.sue_port)));
				
				addrarr->Set(i, addrobj);
				raddr = (struct sockaddr *)(addr6 + 1);
			} else {
				isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Unknown address family")));
			}
		}
		args.GetReturnValue().Set(addrarr);
		usrsctp_freepaddrs(raddrs);
	}
	
	void set_recvbuf(const FunctionCallbackInfo<Value>& args) {
		uint32_t bufsize = args[0]->Uint32Value();
		usrsctp_sysctl_set_sctp_recvspace(bufsize);
	}
	
	void set_sendbuf(const FunctionCallbackInfo<Value>& args) {
		uint32_t bufsize = args[0]->Uint32Value();
		usrsctp_sysctl_set_sctp_sendspace(bufsize);
	}
	
	void finish(const FunctionCallbackInfo<Value>& args) {
		Socket::End(args[0]->ToBoolean()->Value());
	}
	
	void ref(const FunctionCallbackInfo<Value>& args) {
		Socket::ref();
	}
	
	void unref(const FunctionCallbackInfo<Value>& args) {
		Socket::unref();
	}

	void Init(Handle<Object> exports) {
		Socket::Init();
		SocketWrapper::Init(exports);
		NODE_SET_METHOD(exports, "init", init);
		NODE_SET_METHOD(exports, "socket", socket);
		NODE_SET_METHOD(exports, "send", send);
		NODE_SET_METHOD(exports, "bind", bind);
		NODE_SET_METHOD(exports, "bindx", bindx);
		NODE_SET_METHOD(exports, "listen", listen);
		NODE_SET_METHOD(exports, "connect", connect);
		// connectx
		// peeloff
		NODE_SET_METHOD(exports, "close", close);
		NODE_SET_METHOD(exports, "end", end);
		NODE_SET_METHOD(exports, "abort", abort);
		NODE_SET_METHOD(exports, "getLocalAddress", get_local_addr);
		NODE_SET_METHOD(exports, "getRemoteAddress", get_remote_addr);
		NODE_SET_METHOD(exports, "setRecvBufferSize", set_recvbuf);
		NODE_SET_METHOD(exports, "setSendBufferSize", set_sendbuf);
		NODE_SET_METHOD(exports, "finish", finish);
		NODE_SET_METHOD(exports, "ref", ref);
		NODE_SET_METHOD(exports, "unref", unref);
	}

	NODE_MODULE(usrsctp, Init)
	
}

#include <string>
#include <node.h>
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
	void socket(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		
		int domain = args[0]->Uint32Value() == 4 ? AF_INET : AF_INET6;
		
		std::string type_str = std::string(*String::Utf8Value(args[1]->ToString()));
		int type = type_str == "stream" ? SOCK_STREAM : SOCK_SEQPACKET;
		
		Local<Function> cb = Local<Function>::Cast(args[2]);
		Local<Object> obj = sock->GetWrapper()->ToObject();
		obj->Set(String::NewFromUtf8("callback"), cb);
		
		Socket *sock = new Socket(domain, type);
		
		args.GetReturnValue().Set();
	}

	//usrsctp_sendv
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
		
		//
	}

	void Init(Handle<Object> exports) {
		Socket::Init();
		SocketWrapper::Init(exports);
		NODE_SET_METHOD(exports, "init", init);
		NODE_SET_METHOD(exports, "socket", socket);
	}

	NODE_MODULE(usrsctp, Init)
	
}

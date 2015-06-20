#include "socket_wrapper_class.h"

namespace usrsctp {
	using namespace v8;
	using namespace node;
	Persistent<FunctionTemplate> SocketWrapper::constructor_tpl;

	SocketWrapper::SocketWrapper(Socket *sock): sock(sock) {}
	SocketWrapper::~SocketWrapper() {}

	void SocketWrapper::Init(Local<Object> exports) {
		Isolate *isolate = Isolate::GetCurrent();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate);
		tpl->SetClassName(String::NewFromUtf8(isolate, "UsrsctpSocket"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
		constructor_tpl.Reset(isolate, tpl);
		exports->Set(String::NewFromUtf8(isolate, "UsrsctpSocket"), tpl->GetFunction());
	}

	Local<Object> SocketWrapper::ToObject() {
		Isolate *isolate = Isolate::GetCurrent();
		Local<Function> cons = Local<FunctionTemplate>::New(isolate, constructor_tpl)->GetFunction();
		Local<Object> instance = cons->NewInstance();
		Wrap(instance);
		instance->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "usrsctp_socket"));
		return instance;
	}
	
	SocketWrapper *SocketWrapper::FromObject(Local<Value> obj) {
		Isolate *isolate = Isolate::GetCurrent();
		Local<FunctionTemplate> tpl = Local<FunctionTemplate>::New(isolate, constructor_tpl);
		if (!tpl->HasInstance(obj)) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Object is not an instance of usrsctp socket")));
			return nullptr;
		}
		SocketWrapper *sw = ObjectWrap::Unwrap<SocketWrapper>(obj->ToObject());
		if (!sw) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Object contains nullptr")));
			return nullptr;
		}
		return sw;
	}
	
	Socket *SocketWrapper::GetSocket() {
		return sock;
	}
}


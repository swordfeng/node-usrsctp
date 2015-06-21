#include "socket_wrapper_class.h"

namespace usrsctp {
	using namespace v8;
	using namespace node;
	Persistent<FunctionTemplate> SocketWrapper::constructor_tpl;

	void SocketWrapper::Init(Local<Object> exports) {
		Isolate *isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate);
		tpl->SetClassName(String::NewFromUtf8(isolate, "UsrsctpSocket"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
		constructor_tpl.Reset(isolate, tpl);
		exports->Set(String::NewFromUtf8(isolate, "UsrsctpSocket"), tpl->GetFunction());
	}
	
	SocketWrapper *SocketWrapper::FromObject(Local<Value> obj) {
		Isolate *isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
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

	SocketWrapper::SocketWrapper(Socket *sock): sock(sock) {
		Isolate *isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		Local<Function> cons = Local<FunctionTemplate>::New(isolate, constructor_tpl)->GetFunction();
		Local<Object> inst_local = cons->NewInstance();
		Wrap(inst_local);
		inst_local->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "usrsctp_socket"));
		instance.Reset(isolate, inst_local);
	}
	
	SocketWrapper::~SocketWrapper() {
	}

	Socket *SocketWrapper::GetSocket() {
		return sock;
	}

	Local<Object> SocketWrapper::ToObject() {
		Isolate *isolate = Isolate::GetCurrent();
		return Local<Object>::New(isolate, instance);
	}
	
	void SocketWrapper::recv_cb(void *buf, size_t len) {
		// todo
		Isolate *isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		Local<Value> cb_val = Local<Object>::New(isolate, instance)->Get(String::NewFromUtf8("callback"));
		Local<Function> cb = Local<Function>::Cast(cb_val);
		Buffer buf_handle = Buffer::New(String::New(isolate, buf, len));
		const unsigned int argc = 1;
		Local<Value> argv[argc] = { buf_handle };
		cb->Call(isolate->GetCurrentContext()->Global(), argc, argv);
		free(buf);
	}
}


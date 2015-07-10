#include "socket_wrapper_class.h"
#include <node_buffer.h>
#include <iostream>
#include <cstring>
#include <iomanip>
#include <cassert>

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
		Ref();
	}
	
	SocketWrapper::~SocketWrapper() {
		assert(!sock);
	}

	Socket *SocketWrapper::GetSocket() {
		return sock;
	}
	
	void SocketWrapper::SetInvalid() {
		sock = nullptr;
		Unref();
	}

	Local<Object> SocketWrapper::ToObject() {
		return handle();
	}
	
	void SocketWrapper::recv_cb(void *buf, size_t len, int flags, struct sctp_rcvinfo *info, struct sockaddr_storage *addr) {
		Isolate *isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		Local<Value> cb_val = handle()->Get(String::NewFromUtf8(isolate, "onData"));
		Local<Function> cb = Local<Function>::Cast(cb_val);
		// warning: this may not work on other versions of node.js
		// if this problem is critical, it should be changed in the future
		Local<Object> buf_handle = Buffer::New(isolate, static_cast<const char*>(buf), len);
		
		Local<Object> info_obj = Object::New(isolate);
		info_obj->Set(String::NewFromUtf8(isolate, "stream_id"), Number::New(isolate, info->rcv_sid));
		info_obj->Set(String::NewFromUtf8(isolate, "ssn"), Number::New(isolate, info->rcv_ssn));
		info_obj->Set(String::NewFromUtf8(isolate, "ppid"), Number::New(isolate, ntohl(info->rcv_ppid)));
		info_obj->Set(String::NewFromUtf8(isolate, "tsn"), Number::New(isolate, info->rcv_tsn));
		info_obj->Set(String::NewFromUtf8(isolate, "cumtsn"), Number::New(isolate, info->rcv_cumtsn));
		info_obj->Set(String::NewFromUtf8(isolate, "assoc_id"), Number::New(isolate, info->rcv_assoc_id));
		info_obj->Set(String::NewFromUtf8(isolate, "unordered"), Boolean::New(isolate, (info->rcv_flags & SCTP_UNORDERED) != 0));
		info_obj->Set(String::NewFromUtf8(isolate, "completed"), Boolean::New(isolate, (flags & MSG_EOR) != 0));
		
		const unsigned int argc = 2;
		Local<Value> argv[argc] = { buf_handle, info_obj };
		cb->Call(isolate->GetCurrentContext()->Global(), argc, argv);
	}
	
	void SocketWrapper::notif_cb(union sctp_notification *notification) {
		Isolate *isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		
		Local<Value> cb_val = handle()->Get(String::NewFromUtf8(isolate, "onNotification"));
		Local<Function> cb = Local<Function>::Cast(cb_val);
		
		Local<Object> notif_obj = Object::New(isolate);

		char aaddr[50];
		const char *state = "UNKNOWN";
		
#define CASETYPE(tname) case tname: \
	notif_obj->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, #tname));
#define CASESTATE(statename) case statename: state = #statename; break

		switch (notification->sn_header.sn_type) {
			CASETYPE(SCTP_ASSOC_CHANGE)
				switch (notification->sn_assoc_change.sac_state) {
					CASESTATE(SCTP_COMM_UP);
					CASESTATE(SCTP_COMM_LOST);
					CASESTATE(SCTP_RESTART);
					CASESTATE(SCTP_SHUTDOWN_COMP);
					CASESTATE(SCTP_CANT_STR_ASSOC);
				}
				notif_obj->Set(String::NewFromUtf8(isolate, "state"), String::NewFromUtf8(isolate, state));
				notif_obj->Set(String::NewFromUtf8(isolate, "assoc_id"), Number::New(isolate, notification->sn_assoc_change.sac_assoc_id));
				notif_obj->Set(String::NewFromUtf8(isolate, "outbound_streams"), Number::New(isolate, notification->sn_assoc_change.sac_outbound_streams));
				notif_obj->Set(String::NewFromUtf8(isolate, "inbound_streams"), Number::New(isolate, notification->sn_assoc_change.sac_inbound_streams));
				notif_obj->Set(String::NewFromUtf8(isolate, "error"), Number::New(isolate, notification->sn_assoc_change.sac_error));
				break;
			CASETYPE(SCTP_PEER_ADDR_CHANGE)
				if (notification->sn_paddr_change.spc_aaddr.ss_family == AF_INET) {
					inet_ntop(AF_INET, 
						&((struct sockaddr_in *)(&notification->sn_paddr_change.spc_aaddr))->sin_addr,
						aaddr, 50);
				} else if (notification->sn_paddr_change.spc_aaddr.ss_family == AF_INET6) {
					inet_ntop(AF_INET6, 
						&((struct sockaddr_in6 *)(&notification->sn_paddr_change.spc_aaddr))->sin6_addr,
						aaddr, 50);
				}
				switch (notification->sn_paddr_change.spc_state) {
					CASESTATE(SCTP_ADDR_AVAILABLE);
					CASESTATE(SCTP_ADDR_UNREACHABLE);
					CASESTATE(SCTP_ADDR_REMOVED);
					CASESTATE(SCTP_ADDR_ADDED);
					CASESTATE(SCTP_ADDR_MADE_PRIM);
					CASESTATE(SCTP_ADDR_CONFIRMED);
				}
				notif_obj->Set(String::NewFromUtf8(isolate, "address"), String::NewFromUtf8(isolate, aaddr));
				notif_obj->Set(String::NewFromUtf8(isolate, "state"), String::NewFromUtf8(isolate, state));
				notif_obj->Set(String::NewFromUtf8(isolate, "assoc_id"), Number::New(isolate, notification->sn_paddr_change.spc_assoc_id));
				notif_obj->Set(String::NewFromUtf8(isolate, "error"), Number::New(isolate, notification->sn_paddr_change.spc_error));
				break;
			CASETYPE(SCTP_SENDER_DRY_EVENT)
				notif_obj->Set(String::NewFromUtf8(isolate, "assoc_id"), Number::New(isolate, notification->sn_sender_dry_event.sender_dry_assoc_id));
				break;
			CASETYPE(SCTP_SHUTDOWN_EVENT)
				break;
			CASETYPE(SCTP_SEND_FAILED_EVENT)
				break;
			CASETYPE(SCTP_REMOTE_ERROR)
				break;
			CASETYPE(SCTP_ADAPTATION_INDICATION)
				break;
			CASETYPE(SCTP_PARTIAL_DELIVERY_EVENT)
				break;
			CASETYPE(SCTP_AUTHENTICATION_EVENT)
				break;
			CASETYPE(SCTP_NOTIFICATIONS_STOPPED_EVENT)
				break;
		}

#undef CASETYPE
#undef CASESTATE

		const unsigned int argc = 1;
		Local<Value> argv[argc] = { notif_obj };
		cb->Call(isolate->GetCurrentContext()->Global(), argc, argv);
	}
}


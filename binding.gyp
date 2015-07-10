{
	"targets": [
		{
			"target_name": "usrsctp",
			'dependencies': [
				'usrsctplib.gyp:usrsctplib',
			],
			'cflags': [
				'-std=c++11',
			],
			'defines': [
				'INET=1',
				'INET6=1',
			],
			'include_dirs': [
				'usrsctp/usrsctplib',
			],
			'libraries': [
				'-lpthread',
			],
			"sources": [ 
				"usrsctp.cc", 
				"socket_wrapper_class.cc", 
				"socket_class.cc",
			]
		}
	]
}

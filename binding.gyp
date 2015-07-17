{
	"targets": [
		{
			"target_name": "usrsctp",
			'dependencies': [
				'usrsctplib.gyp:usrsctplib',
			],
			'defines': [
				'INET=1',
				'INET6=1',
				'__Userspace__',
			],
			'include_dirs': [
				'usrsctp/usrsctplib',
			],
			"sources": [ 
				"src/usrsctp.cc", 
				"src/socket_wrapper_class.cc", 
				"src/socket_class.cc",
			],
			'conditions':[
				['OS=="win"', {
					'libraries': [
						'Ws2_32.lib',
					],
				}, { # OS!="win"
					'cflags': [
						'-std=c++11',
					],
					'libraries': [
						'-lpthread',
					],
				}],
			]
		}
	]
}

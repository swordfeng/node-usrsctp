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
					'libraries': [
						'-lpthread',
					],
				}],
			]
		}
	]
}

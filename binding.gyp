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
					'defines': [
						'WINVER=0x0502',
						'_WIN32_WINNT=0x0502',
					],
					'defines!': [
						'WINVER=0x0602',
						'_WIN32_WINNT=0x0602',
					],
					'cflags!': [ '/W3', '/WX' ],
					'cflags': [ '/w' ],
					'msvs_disabled_warnings': [ 4002, 4013, 4018, 4133, 4267, 4313, 4700 ],
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

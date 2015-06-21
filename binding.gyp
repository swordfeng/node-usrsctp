{
	"targets": [
		{
			"target_name": "usrsctp",
			'cflags': [
				'-std=c++11',
				'-DINET=1',
				'-DINET6=1',
			],
			'include_dirs': [
				'./usrsctp/usrsctplib',
			],
			'libraries': [
				"-L../usrsctp/usrsctplib/.libs", '-lusrsctp', '-lpthread',
			],
			"sources": [ "usrsctp.cc", "socket_wrapper_class.cc", "socket_class.cc"  ]
		}
	]
}

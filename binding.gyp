{
	"targets": [
		{
		"target_name": "nodeleap",
		"cflags!": [ "-fno-exceptions" ],
		"cflags": [ "-std=c++11" ],
		"cflags_cc!": [ "-fno-exceptions" ],
		"sources": [ "binding.cc" ],
		"include_dirs": [
			"<!@(node -p \"require('node-addon-api').include\")",
			"C:\\Program Files\\Ultraleap\\LeapSDK\\include"
		],
		"libraries": [
			"C:\\Program Files\\Ultraleap\\LeapSDK\\lib\\x64\\LeapC.lib"
		],
		"copies": [{
				"destination": "<(module_root_dir)/build/Release/",
					"files": [
						"C:\\Program Files\\Ultraleap\\LeapSDK\\lib\\x64\\LeapC.dll",
					]
		}],
		"defines": [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
		}
	]
}

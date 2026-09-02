#include "MyObject.h"
#include "TestObject.h"
#include "ResourceServer.h"
#include "webbridge/Object.h"
#include "webbridge/error.h"
#include <webview/webview.h>
#include <portable-file-dialogs.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>

int WINAPI WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/,
	LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	//AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	
	// Open timing log file
	std::ofstream timing_log("timing_trace.log", std::ios::app);
	std::streambuf* cout_backup = std::cout.rdbuf();
	std::cout.rdbuf(timing_log.rdbuf());

	try {
		// Start HTTP server with embedded resources
		ResourceServer server;
		if (!server.start()) {
			std::cerr << "Failed to start resource server\n";
			return 1;
		}
		
		std::cout << "Resource server running on " << server.get_url() << "\n";
		
#ifdef _DEBUG
		webview::webview w(true, nullptr);
#else
		webview::webview w(false, nullptr);

		// Disable context menu in release mode
		w.eval(R"(
			document.addEventListener('contextmenu', (e) => {
				e.preventDefault();
				return false;
			});
		)");
#endif
		w.set_title("WebBridge Demo");
		w.set_size(900, 700, WEBVIEW_HINT_NONE);

		// Register error handler
		webbridge::set_error_handler([](webbridge::error& err, const std::exception& ex) {
			std::cout << "Error " << err.code << ": " << err.message << "\n";
		});
		
		// Register type -> needs to be created in js
		webbridge::register_type<MyObject>(&w);
		webbridge::register_type<TestObject>(&w);

		// Navigate FIRST so the frontend (with WebbridgeRuntime) is loaded
		w.navigate(server.get_url());
		w.run();
		
		// Restore cout and close timing log
		std::cout.rdbuf(cout_backup);
		timing_log.close();
	} catch (const webview::exception &e) {
		std::cerr << e.what() << '\n';
		return 1;
	}

	return 0;
}

#pragma once

#include <httplib.h>
#include <cmrc/cmrc.hpp>
#include <string>
#include <thread>
#include <atomic>
#include <map>
#include <iostream>

CMRC_DECLARE(frontend);

// MIME type detection based on file extension
inline std::string get_mime_type(const std::string& path) {
	static const std::map<std::string, std::string> mime_types = {
		{".html", "text/html; charset=utf-8"},
		{".css", "text/css; charset=utf-8"},
		{".js", "application/javascript; charset=utf-8"},
		{".json", "application/json; charset=utf-8"},
		{".png", "image/png"},
		{".jpg", "image/jpeg"},
		{".jpeg", "image/jpeg"},
		{".svg", "image/svg+xml"},
		{".ico", "image/x-icon"},
		{".woff", "font/woff"},
		{".woff2", "font/woff2"},
		{".ttf", "font/ttf"},
		{".eot", "application/vnd.ms-fontobject"}
	};
	
	auto dot_pos = path.find_last_of('.');
	if (dot_pos != std::string::npos) {
		std::string ext = path.substr(dot_pos);
		auto it = mime_types.find(ext);
		if (it != mime_types.end()) {
			return it->second;
		}
	}
	return "application/octet-stream";
}

// HTTP Server serving embedded resources from cmrc filesystem
class ResourceServer {
public:
	ResourceServer() : running_(false), port_(0) {}
	
	bool start() {
		auto fs = cmrc::frontend::get_filesystem();
		server_.Get("/.*", [fs](const httplib::Request& req, httplib::Response& res) {
			std::string path = req.path;

			if (path == "/" || path.empty()) {
				path = "/index.html";
			}

			if (!path.empty() && path[0] == '/') {
				path = path.substr(1);
			}
			
			try {
				if (fs.exists(path)) {
					auto file = fs.open(path);
					std::string content(file.begin(), file.end());
					
					res.set_content(content, get_mime_type(path));
				} else {
					res.status = 404;
					res.set_content("Not Found", "text/plain");
				}
			} catch (const std::exception& e) {
				res.status = 500;
				res.set_content(std::string("Internal Server Error: ") + e.what(), "text/plain");
			}
		});

		port_ = server_.bind_to_any_port("127.0.0.1");
		if (port_ == -1) {
			std::cerr << "Failed to bind server to port\n";
			return false;
		}

		running_ = true;
		server_thread_ = std::thread([this]() {
			server_.listen_after_bind();
		});
		
		return true;
	}
	
	void stop() {
		if (running_) {
			running_ = false;
			server_.stop();
			if (server_thread_.joinable()) {
				server_thread_.join();
			}
		}
	}
	
	int get_port() const { return port_; }
	
	std::string get_url() const {
		return "http://127.0.0.1:" + std::to_string(port_);
	}
	
	~ResourceServer() {
		stop();
	}
	
private:
	httplib::Server server_;
	std::thread server_thread_;
	std::atomic<bool> running_;
	int port_;
};

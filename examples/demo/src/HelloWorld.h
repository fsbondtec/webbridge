#pragma once

#include "webbridge/object.h"

#include <string>

// Minimal example showing the four building blocks WebBridge exposes to JS:
// properties, methods (sync + async), events, and constants.
class HelloWorld : public webbridge::object
{
public:
	// Property: value lives in C++, exposed as a reactive Svelte store in JS.
	// Read it in C++ with the parenthesis operator: message()
	property<std::string> message{ "Hello, World!" };
	property<int> greetCount{ 0 };

	// Event: JS equivalent of a Qt signal / Node EventEmitter.
	event<std::string> greeted;

	// Instance constant: read-only in JS, set once at construction.
	const std::string author;

public:
	explicit HelloWorld(const std::string& author_) : author(author_) {}

	// Synchronous method: runs on the call thread in C++.
	// JS still gets a Promise, since C++ <-> JS calls are always async there.
	void greet(const std::string& name);

	// [[async]] method: runs on a worker thread in C++ so it never blocks
	// the main/UI thread, even for slow operations.
	[[async]] void slowGreet(const std::string& name);
};

// convenience method
#include "HelloWorld_registration.h"

namespace webbridge {
template<> inline void register_type<HelloWorld>(webview::webview* w) {
	impl::register_HelloWorld(w);
}}

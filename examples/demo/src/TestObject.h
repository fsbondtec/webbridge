#pragma once

#include "webbridge/object.h"
#include <string>

class TestObject : public webbridge::object
{
public:
    // Property
    property<std::string> testProp{"Initial Value"};
    
    // Event
    event<std::string> testEvent;

public:
    TestObject() = default;

    // Async Methode
    [[async]] void asyncProcess(const std::string& input);

    // Sync Methode
    int syncCalculate(int value);

    // Benchmark: Minimale Sync-Methode (triviale Rechnung)
    int benchmarkSync(int x) { return x + 1; }

    // Benchmark: Minimale Async-Methode (triviale Rechnung)
    [[async]] int benchmarkAsync(int x);

    // JSON Deserialisierung Benchmark
    [[async]] double jsonBench();

    // JSON Array Benchmark
    [[async]] double jsonBench2();
};

// convenience method
#include "TestObject_registration.h"

namespace webbridge {
template<> inline void register_type<TestObject>(webview::webview* w) {
	impl::register_TestObject(w);
}}

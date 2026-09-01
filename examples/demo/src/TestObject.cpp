#include "TestObject.h"
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

void TestObject::asyncProcess(const std::string& input)
{
    // Simuliere asynchrone Operation
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    testProp = "Processed: " + input;
    testEvent.emit("Processing completed for: " + input);
}

int TestObject::syncCalculate(int value)
{
    return value * 2 + 10;
}

int TestObject::benchmarkAsync(int x)
{
    return x + 1;
}

double TestObject::jsonBench()
{
    // JSON String ähnlich wie multiParamTest Parameter
    std::string jsonStr = R"({
        "intValue": 999,
        "boolValue": true,
        "strValue": "Test String from JSON",
        "vecValue": [10, 20, 30, 40, 50],
        "statusValue": 1,
        "podValue": {"a": 777, "b": 888888888}
    })";

    std::string* vjsonStrPtr = &jsonStr;
    volatile int preventOptimization = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        std::string jsonCopy = *vjsonStrPtr;
        auto j = nlohmann::json::parse(jsonCopy);

        // Werte extrahieren um sicherzustellen, dass der Compiler es nicht wegoptimiert
        preventOptimization += j["intValue"].get<int>();
        preventOptimization += j["boolValue"].get<bool>() ? 1 : 0;
        preventOptimization += j["strValue"].get<std::string>().size();
        preventOptimization += j["vecValue"].size();
        preventOptimization += j["statusValue"].get<int>();
        preventOptimization += j["podValue"]["a"].get<int>();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start);

    // Verwende preventOptimization um sicherzustellen, dass es nicht wegoptimiert wird
    if (preventOptimization == 0) {
        return -1.0; // Wird nie passieren, aber verhindert Optimierung
    }

    return duration.count();
}

double TestObject::jsonBench2()
{
    // JSON Array mit Werten wie in jsonBench
    std::string jsonStr = R"([
        999,
        true,
        "Test String from JSON",
        [10, 20, 30, 40, 50],
        1,
        {"a": 777, "b": 888888888}
    ])";

    std::string* vjsonStrPtr = &jsonStr;
    volatile int preventOptimization = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        std::string jsonCopy = *vjsonStrPtr;
        auto arr = nlohmann::json::parse(jsonCopy);
        // Extrahiere Werte wie in jsonBench
        preventOptimization += arr[0].get<int>(); // intValue
        preventOptimization += arr[1].get<bool>() ? 1 : 0; // boolValue
        preventOptimization += arr[2].get<std::string>().size(); // strValue
        preventOptimization += arr[3].size(); // vecValue
        preventOptimization += arr[4].get<int>(); // statusValue
        preventOptimization += arr[5]["a"].get<int>(); // podValue.a
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start);

    if (preventOptimization == 0) {
        return -1.0;
    }
    return duration.count();
}

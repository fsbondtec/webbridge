#pragma once

#include "webbridge/object.h"

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct Pod { 
	unsigned a; unsigned long long b; 
	auto operator<=>(const Pod&) const = default;
};

inline void to_json(nlohmann::json& j, const Pod& p) {
	j = nlohmann::json{{"a", p.a}, {"b", p.b}};
}

inline void from_json(const nlohmann::json& j, Pod& p) {
	j.at("a").get_to(p.a);
	j.at("b").get_to(p.b);
}

class MyObject : public webbridge::object
{
public:
	enum class Status {
		Idle,
		Running,
		Completed,
		Error
	};

	property<bool> aBool;
	property<std::string> strProp;
	property<int> counter{ 0 };
	property<std::vector<int>> numbers;
	property<Status> status{ Status::Idle };
	property<Pod> pod{ Pod{0,0} };
	event<int, bool> aEvent;

	const std::string version;
	static inline const std::string appversion{"myapp 1.0"};
	static inline constexpr unsigned CPP_VERSION{23};

public:
	explicit MyObject(const std::string& version_) : version(version_) {}
	virtual ~MyObject() {
		std::ignore = 6;
	}

	[[async]] void foo(const std::string& val);

	bool bar();

	[[async]] void file();

	void testVectors();

	void throwError();

	std::string multiParamTest(
		int intValue,
		bool boolValue,
		const std::string& strValue,
		const std::vector<int>& vecValue,
		Status statusValue,
		const Pod& podValue
	);
};

// convenience method
#include "MyObject_registration.h"

namespace webbridge {
template<> inline void register_type<MyObject>(webview::webview* w) {
	impl::register_MyObject(w);
}}

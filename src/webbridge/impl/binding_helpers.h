#pragma once

/**
 * WebBridge Binding Helpers
 * 
 * Reusable template functions for binding C++ objects to JavaScript.
 * Used by auto-generated registration files.
 */

#include <webview/webview.h>
#include <memory>
#include <thread>
#include <tuple>
#include <functional>
#include <type_traits>
#include <stdexcept>
#include <typeinfo>
#include <nlohmann/json.hpp>

#include "object_registry.h"
#include "property_impl.h"
#include "event_impl.h"
#include "concepts.h"
#include "../object.h"
#include "../error.h"

namespace webbridge::impl {

// =============================================================================
// Central Error Handling
// =============================================================================

/**
 * Executes a callable, serializes result to JSON, catches all exceptions.
 * Returns tuple<status_code, json_string>.
 * 
 * Success: {0, <value>}        - Direct value for clean JS API
 * Error:   {code, {"error":...}} - Structured error object
 */
template<typename Callable, typename... Args>
std::tuple<int, std::string> invoke_and_serialize(Callable&& func, Args&&... args) {
	try {
		using result_type = std::invoke_result_t<Callable, Args...>;
		
		if constexpr (std::is_void_v<result_type>) {
			std::invoke(std::forward<Callable>(func), std::forward<Args>(args)...);
			return {0, "null"};
		} else {
			auto result = std::invoke(std::forward<Callable>(func), std::forward<Args>(args)...);
			return {0, nlohmann::json(result).dump()};
		}
	}
	catch (const nlohmann::json::exception& ex) {
		auto err = from_json_exception(ex);
		return {err.code, err.dump()};
	}
	catch (const std::exception& ex) {
		auto err = from_cpp_exception(ex, RUNTIME_ERROR);
		return {err.code, err.dump()};
	}
	catch (...) {
		auto err = unknown_error();
		return {err.code, err.dump()};
	}
}

// =============================================================================
// Object Lookup Helper
// =============================================================================

template<typename TObj>
std::shared_ptr<TObj> get_object_or_throw(object_registry& registry, const std::string& object_id) {
	auto obj = registry.get<TObj>(object_id);
	if (!obj) {
		throw std::runtime_error("Object not found: " + object_id);
	}
	return obj;
}

// =============================================================================
// Property Subscription
// =============================================================================

template<typename T>
void subscribe_property(
	webview::webview& w_ref,
	const std::string& object_id,
	const std::string& prop_name,
	property<T>& prop)
{
	prop.set_on_changed([&w_ref, object_id, prop_name](const T& val) {
		w_ref.dispatch([&w_ref, object_id, prop_name, val]() {
			w_ref.eval(std::format(
				"window.__webbridge_notify('{}', '{}', {})",
				object_id, prop_name, nlohmann::json(val).dump()
			));
		});
	});
}

// =============================================================================
// Event Subscription
// =============================================================================

template<typename... Args>
void subscribe_event(
	webview::webview& w_ref,
	const std::string& object_id,
	const std::string& event_name,
	event<Args...>& evt)
{
	evt.set_forwarder([&w_ref, object_id, event_name](Args... args) {
		w_ref.dispatch([&w_ref, object_id, event_name, args...]() {
			std::string args_str;
			((args_str += nlohmann::json(args).dump() + ", "), ...);
			if (!args_str.empty()) {
				args_str.resize(args_str.size() - 2);
			}
			w_ref.eval(std::format(
				"window.__webbridge_emit('{}', '{}', {})",
				object_id, event_name, args_str
			));
		});
	});
}

} // namespace webbridge::impl

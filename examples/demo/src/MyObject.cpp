#include "MyObject.h"
#include <portable-file-dialogs.h>

void MyObject::foo(const std::string& val)
{
	strProp = val;
	aEvent.emit(42, false);
}

bool MyObject::bar()
{
	Pod newPod;
	newPod.a = 42;
	newPod.b = 123456789ULL;
	pod = newPod;
	
	return !aBool();
}

void MyObject::file()
{
	auto selection = pfd::open_file("Choose file").result();
}

void MyObject::testVectors()
{
	status = Status::Running;
	std::vector<int> fibonacci = { 1, 1, 2, 3, 5, 8, 13, 21 };
	numbers = fibonacci;
	strProp = "Vector mit " + std::to_string(fibonacci.size()) + " Fibonacci-Zahlen";
	counter = static_cast<int>(fibonacci.size());
	status = Status::Completed;
}

void MyObject::throwError()
{
	throw std::runtime_error("Test Exception");
}

std::string MyObject::multiParamTest(
	int intValue,
	bool boolValue,
	const std::string& strValue,
	const std::vector<int>& vecValue,
	Status statusValue,
	const Pod& podValue)
{
	counter = intValue;
	aBool = boolValue;
	strProp = strValue;
	numbers = vecValue;
	status = statusValue;
	pod = podValue;
	
	std::string result = "multiParamTest called with: ";
	result += "int=" + std::to_string(intValue);
	result += ", bool=" + std::string(boolValue ? "true" : "false");
	result += ", str='" + strValue + "'";
	result += ", vec.size=" + std::to_string(vecValue.size());
	result += ", status=" + std::to_string(static_cast<int>(statusValue));
	result += ", pod.a=" + std::to_string(podValue.a);
	result += ", pod.b=" + std::to_string(podValue.b);
	
	return result;
}

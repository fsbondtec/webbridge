#include "HelloWorld.h"

#include <chrono>
#include <thread>

void HelloWorld::greet(const std::string& name)
{
	message = "Hello, " + name + "!";
	greetCount = greetCount() + 1;
	greeted.emit(name);
}

void HelloWorld::slowGreet(const std::string& name)
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	greet(name);
}

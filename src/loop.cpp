#include "loop.hpp"
#include <syslog.h>
#include <chrono>
#include <syslog.h>

int Loop::start()
{
	//do init RF


	//start thread
	run_=true;
	thread_ = std::thread(&Loop::loop, this);

	return 0;
}

void Loop::stop()
{
	run_=false;
	thread_.join();
}

void Loop::loop()
{

	syslog (LOG_INFO, "loop init");
	while(run_)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	syslog (LOG_INFO, "loop deinit");

}

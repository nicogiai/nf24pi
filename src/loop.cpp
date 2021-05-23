#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "loop.hpp"
#include <syslog.h>
#include <chrono>
#include <syslog.h>

#if HAVE_LIBRF24
# include <RF24/RF24.h> // RF24, RF24_PA_LOW, delay()
#endif

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

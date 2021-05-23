#include "loop.hpp"
#include <syslog.h>
#include <chrono>
#include <syslog.h>

int Loop::start()
{
	//do init RF
#if HAVE_LIBRF24
	radio_ = new RF24(22, 0);
	if (!radio_->begin()) {
		return 1;
	}

    // to use different addresses on a pair of radios, we need a variable to
    // uniquely identify which address this radio will use to transmit
    bool radioNumber = 1; // 0 uses address[0] to transmit, 1 uses address[1] to transmit

    // Let these addresses be used for the pair
    uint8_t address[2][6] = {"1Node", "2Node"};
    // It is very helpful to think of an address as a path instead of as
    // an identifying device destination

    // save on transmission time by setting the radio to only transmit the
    // number of bytes we need to transmit a float
    radio_->setPayloadSize(sizeof(payload)); // float datatype occupies 4 bytes

    // Set the PA Level low to try preventing power supply related problems
    // because these examples are likely run with nodes in close proximity to
    // each other.
    radio_->setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.

    // set the TX address of the RX node into the TX pipe
    radio_->openWritingPipe(address[radioNumber]);     // always uses pipe 0

    // set the RX address of the TX node into a RX pipe
    radio_->openReadingPipe(1, address[!radioNumber]); // using pipe 1
    syslog (LOG_INFO, "listening on %s",address[!radioNumber]);

#endif

	//start thread
	run_=true;
	thread_ = std::thread(&Loop::loop, this);

	return 0;
}

void Loop::stop()
{
	run_=false;
	thread_.join();

#if HAVE_LIBRF24
	delete radio_;
#endif
}

void Loop::loop()
{
	syslog (LOG_INFO, "loop init");

#if HAVE_LIBRF24
	uint8_t pipe;
	radio_->startListening();                                  // put radio in RX mode
#endif

	while(run_)
	{
#if HAVE_LIBRF24
		if (radio_->available(&pipe)) {                        // is there a payload? get the pipe number that recieved it
			uint8_t bytes = radio_->getPayloadSize();          // get the size of the payload
			uint8_t bpayload[bytes];

			radio_->read(bpayload, bytes);                     // fetch payload from FIFO
			syslog (LOG_INFO, "bytes received %d",bytes);

			//cout << "Received " << (unsigned int)bytes;      // print the size of the payload
			//cout << " bytes on pipe " << (unsigned int)pipe; // print the pipe number
			//cout << ": " << payload << endl;                 // print the payload's value

		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
#else
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
#endif
	}

#if HAVE_LIBRF24
	radio_->stopListening();
#endif

	syslog (LOG_INFO, "loop deinit");

}

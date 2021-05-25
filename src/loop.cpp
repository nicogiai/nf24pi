#include "loop.hpp"
#include <syslog.h>
#include <chrono>
#include <syslog.h>
#include <spdlog/spdlog.h>
//#include <mosquittopp.h>
#include <mosquitto.h>

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
    radio_->setPayloadSize(sizeof(payload)); // float datatype occupies 8 bytes (temp + humidity)

    // Set the PA Level low to try preventing power supply related problems
    // because these examples are likely run with nodes in close proximity to
    // each other.
    radio_->setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.

    // set the TX address of the RX node into the TX pipe
    radio_->openWritingPipe(address[radioNumber]);     // always uses pipe 0

    // set the RX address of the TX node into a RX pipe
    radio_->openReadingPipe(1, address[!radioNumber]); // using pipe 1

    spdlog::get(PACKAGE_NAME)->info("listening on: {}", address[!radioNumber]));

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
#if HAVE_LIBRF24
	uint8_t pipe;
	radio_->startListening();                                  // put radio in RX mode
#endif

	auto logger = spdlog::get(PACKAGE_NAME);

	mosq = mosquitto_new(NULL, true, NULL);
	if(mosquitto_connect(mosq, "127.0.0.1", 1883, 60)){
		logger->critical("Unable to connect mosquitto broker");
		exit(1);
	}
	int loop = mosquitto_loop_start(mosq);
	if(loop != MOSQ_ERR_SUCCESS){
		logger->critical("Unable to start mosquitto loop");
		exit(1);
	}
	std::string temp_str;
	std::string humidity_str;

	while(run_)
	{
#if HAVE_LIBRF24
		if (radio_->available(&pipe)) {                        // is there a payload? get the pipe number that recieved it
			uint8_t bytes = radio_->getPayloadSize();          // get the size of the payload
			radio_->read(payload, bytes);                     // fetch payload from FIFO
			//cout << "Received " << (unsigned int)bytes;      // print the size of the payload
			//cout << " bytes on pipe " << (unsigned int)pipe; // print the pipe number
			//cout << ": " << payload << endl;                 // print the payload's value
			logger->debug("pipe {}: temp {} ºC, humidity {} %", (unsigned int)pipe, payload[0], payload[1]); // log data

			temp_str = std::to_string(payload[0]);
			mosquitto_publish(mosq, NULL, "dht22/temp", temp_str.size(), temp_str.c_str(), 0, 0);

			humidity_str = std::to_string(payload[1]);
			mosquitto_publish(mosq, NULL, "dht22/humidity", humidity_str.size(), humidity_str.c_str(), 0, 0);
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

	mosquitto_disconnect(mosq);
	mosquitto_loop_stop(mosq,false);

}

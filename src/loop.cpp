#include "loop.hpp"

#if HAVE_CONFIG_H
# include <config.h>
#endif


#if HAVE_LIBRF24
# include <RF24/RF24.h> // RF24, RF24_PA_LOW, delay()
#endif

#include <chrono>
#include <spdlog/spdlog.h>
#include <mosquitto.h>
#include <csignal>
#include <cstdio>
#include <curl/curl.h>
#include <sstream>

/*
 * curl -i -XPOST 'http://localhost:8086/write?db=mydb'
--data-binary 'cpu_load_short,host=server01,region=us-west value=0.64 1434055562000000000'
 */
int Loop::write_influxdb(std::string var, float value, int id)
{
  CURL *curl;
  CURLcode res;
  const auto now = std::chrono::system_clock::now();
  auto timestamp_now = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

  auto format = "%s,host=raspberrypi,sensorid=%d value=%.1f %d";
  auto size = std::snprintf(nullptr, 0, format, var, id, value, timestamp_now);
  std::string query(size + 1, '\0');
  std::sprintf(&query[0], format, var, id, value, timestamp_now);

  //std::ostringstream query;
  //query << var << ",host=raspberrypi,sensorid=" << id << " value=" << value << " " << timestamp_now;
  //std::string query_str = string_format("%s,host=raspberrypi,sensorid=%d value=%.1f %d", var,value,id,timestamp_now);

  //static const char *postthis = "moo mooo moo moo";

  spdlog::get(PACKAGE_NAME)->info("query: {}", query);

  /* In windows, this will init the winsock stuff */
  curl_global_init(CURL_GLOBAL_ALL);

  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {
    /* First set the URL that is about to receive our POST. This URL can
       just as well be a https:// URL if that is what should receive the
       data. */
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8086/write?db=nf24pi");
    /* Now specify the POST data */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, query.c_str());

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK) {
      spdlog::get(PACKAGE_NAME)->critical("curl_easy_perform() failed: {}", curl_easy_strerror(res));
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return 0;
}

void Loop::start()
{
	//start thread
	run_=true;
	thread_ = std::thread(&Loop::loop, this);
}

void Loop::stop()
{
	run_=false;
	thread_.join();
}

void Loop::loop()
{
	float payload[2];

#if HAVE_LIBRF24
	RF24 *radio_ = new RF24(22, 0);
	uint8_t pipe;

	try
	{
		radio_->begin();

		// to use different addresses on a pair of radios, we need a variable to
		// uniquely identify which address this radio will use to transmit
		//uint8_t radioNumber = 1; // 0 uses address[0] to transmit, 1 uses address[1] to transmit

		// Let these addresses be used for the pair
		uint8_t address[3][6] = {"1Node", "2Node","3Node"};
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
		radio_->openWritingPipe(address[2]);

		// set the RX address of the TX node into a RX pipe
		radio_->openReadingPipe(1, address[0]); // using pipe 1
		radio_->openReadingPipe(2, address[1]); // using pipe 2

		spdlog::get(PACKAGE_NAME)->info("listening on: {}", address[0]);
		spdlog::get(PACKAGE_NAME)->info("listening on: {}", address[1]);


		radio_->startListening(); // put radio in RX mode

	}
	catch (const std::runtime_error& error)
	{
		spdlog::get(PACKAGE_NAME)->critical("librf24 not root");
		run_=false;
		std::raise(SIGTERM); //error critico.termina la ejecucion del programa
		return;
	}
#else
	spdlog::get(PACKAGE_NAME)->warn("librf24 not present!");
#endif

	auto logger = spdlog::get(PACKAGE_NAME);

	mosq = mosquitto_new(NULL, true, NULL);
	if(mosquitto_connect(mosq, "127.0.0.1", 1883, 60)){
		logger->critical("Unable to connect mosquitto broker");

		run_=false;
		std::raise(SIGTERM); //error critico.termina la ejecucion del programa
		return;
	}

	int loop = mosquitto_loop_start(mosq);
	if(loop != MOSQ_ERR_SUCCESS){
		logger->critical("Unable to start mosquitto loop");

		run_=false;
		std::raise(SIGTERM); //error critico.termina la ejecucion del programa
		return;
	}

	std::string temp_topic_str;
	std::string temp_str;
	std::string humidity_str;
	std::string humidity_topic_str;

	logger->debug("topic   example: {}", string_format("sensor/%d/temperature", 0));
	logger->debug("message example: {}", string_format("%.1f", 1.5));

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

			//form1 mensaje de temperatura y publica
			temp_topic_str = string_format("sensor/%d/temperature", pipe);
			temp_str = string_format("%.1f", payload[0]);
			mosquitto_publish(mosq, NULL, temp_topic_str.c_str(), temp_str.size(), temp_str.c_str(), 0, 0);
			write_influxdb("temperature", payload[0], (int)pipe);

			//form1 mensaje de humedad y publica
			humidity_topic_str = string_format("sensor/%d/humidity", pipe);
			humidity_str = string_format("%.1f", payload[1]);
			mosquitto_publish(mosq, NULL, humidity_topic_str.c_str(), humidity_str.size(), humidity_str.c_str(), 0, 0);
			write_influxdb("humidity", payload[1], (int)pipe);
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
	delete radio_;
#endif

	mosquitto_disconnect(mosq);
	mosquitto_loop_stop(mosq,false);

}

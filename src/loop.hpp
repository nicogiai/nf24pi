/*
 * loop.hpp
 *
 *  Created on: May 23, 2021
 */

#ifndef SRC_LOOP_HPP_
#define SRC_LOOP_HPP_

#if HAVE_CONFIG_H
# include <config.h>
#endif


#if HAVE_LIBRF24
# include <RF24/RF24.h> // RF24, RF24_PA_LOW, delay()
#endif

#include <thread>         // std::thread

class Loop {
	public:
		int start();
		void stop();
	private:
		std::thread thread_;
		bool run_;
		float payload;
#if HAVE_LIBRF24
		RF24 *radio_;
#endif
	private:
		void loop();
};


#endif /* SRC_LOOP_HPP_ */

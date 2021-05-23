/*
 * loop.hpp
 *
 *  Created on: May 23, 2021
 */

#ifndef SRC_LOOP_HPP_
#define SRC_LOOP_HPP_

#include <thread>         // std::thread

class Loop {
	public:
		int start();
		void stop();
	private:
		std::thread thread_;
		bool run_;
	private:
		void loop();
};


#endif /* SRC_LOOP_HPP_ */

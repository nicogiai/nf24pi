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
#include <memory>
#include <string>
#include <stdexcept>

template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    auto buf = std::make_unique<char[]>( size );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

class Loop {
	public:
		int start();
		void stop();
	private:
		std::thread thread_;
		bool run_;
#if HAVE_LIBRF24
		RF24 *radio_;
#endif
		struct mosquitto *mosq;
	private:
		void loop();
};


#endif /* SRC_LOOP_HPP_ */

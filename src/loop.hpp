/*
 * loop.hpp
 *
 *  Created on: May 23, 2021
 */

#ifndef SRC_LOOP_HPP_
#define SRC_LOOP_HPP_

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
		void start();
		void stop();
	private:
		std::thread thread_;
		bool run_;
		struct mosquitto *mosq;
	private:
		void loop();
		int write_influxdb(std::string var, float value, int id);
};


#endif /* SRC_LOOP_HPP_ */

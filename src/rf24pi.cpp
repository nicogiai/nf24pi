#include <iostream>    // cin, cout, endl
#include <syslog.h>

int main(void) {


	std::cout << "Hello from autotools!" << std::endl;
	syslog (LOG_INFO, "Hello from autotools!");

	return 0;

}

#include <chrono>
#include <thread>`

void usleep(unsigned int usec)
{
	std::this_thread::sleep_for(std::chrono::microseconds(usec));
}

#include <windows.h>

//#include <chrono>
//#include <thread>`

HANDLE hTimer = INVALID_HANDLE_VALUE;

void usleep(unsigned int usec)
{
	if (hTimer == INVALID_HANDLE_VALUE)
	{
		hTimer = CreateWaitableTimerW(NULL, TRUE, NULL);
	}
	LARGE_INTEGER waitTime;
	waitTime.QuadPart = -(LONGLONG)usec * 100;  //negative wait time indicates relative time
	SetWaitableTimer(hTimer, &waitTime, 0, NULL, NULL, FALSE);
	
	int maxMilliseconds = (usec + 999) / 1000;
	
	WaitForSingleObject(hTimer, maxMilliseconds);
	
	//std::this_thread::sleep_for(std::chrono::microseconds(usec));
}

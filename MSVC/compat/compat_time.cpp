#include <chrono>
#include <thread>`


void usleep(unsigned int usec)
{
    std::this_thread::sleep_for(std::chrono::microseconds(usec));
}
int clock_gettime(int X, struct timespec* tv)
{
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    std::chrono::nanoseconds nanoseconds_in = std::chrono::duration_cast<std::chrono::nanoseconds>(now);
    std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(now);
    std::chrono::nanoseconds nanoseconds = nanoseconds_in - seconds;
    tv->tv_sec = (time_t)seconds.count();
    tv->tv_nsec = (long)nanoseconds.count();
    return 0;
}

/*

#include <Windows.h>
#include <time.h>

LARGE_INTEGER
getFILETIMEoffset()
{
    SYSTEMTIME s;
    FILETIME f;
    LARGE_INTEGER t;

    s.wYear = 1970;
    s.wMonth = 1;
    s.wDay = 1;
    s.wHour = 0;
    s.wMinute = 0;
    s.wSecond = 0;
    s.wMilliseconds = 0;
    SystemTimeToFileTime(&s, &f);
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;
    return (t);
}

int
clock_gettime(int X, struct timespec* tv)
{
    LARGE_INTEGER           t;
    FILETIME            f;
    double                  nanoseconds;
    static LARGE_INTEGER    offset;
    static double           frequencyToMicroseconds;
    static int              initialized = 0;
    static BOOL             usePerformanceCounter = 0;

    if (!initialized) {
        LARGE_INTEGER performanceFrequency;
        initialized = 1;
        usePerformanceCounter = QueryPerformanceFrequency(&performanceFrequency);
        if (usePerformanceCounter) {
            QueryPerformanceCounter(&offset);
            frequencyToMicroseconds = (double)performanceFrequency.QuadPart / 1000000.;
        }
        else {
            offset = getFILETIMEoffset();
            frequencyToMicroseconds = 10.;
        }
    }
    if (usePerformanceCounter) QueryPerformanceCounter(&t);
    else {
        GetSystemTimeAsFileTime(&f);
        t.QuadPart = f.dwHighDateTime;
        t.QuadPart <<= 32;
        t.QuadPart |= f.dwLowDateTime;
    }

    t.QuadPart -= offset.QuadPart;
    nanoseconds = (double)t.QuadPart / frequencyToMicroseconds * 1000.0;
    t.QuadPart = nanoseconds;
    tv->tv_sec = t.QuadPart / 1000000000;
    tv->tv_nsec = t.QuadPart % 1000000000;

    //tv->tv_usec = t.QuadPart % 1000000;
    return (0);
}


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
*/

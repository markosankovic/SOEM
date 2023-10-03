#include <iostream>
#include <array>

#include "ethercat.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

LARGE_INTEGER prevTime;
LARGE_INTEGER frequency;

char IOmap[4096];

std::array<int, 1000> cnts = {};

VOID CALLBACK TimerCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);

	ec_send_processdata();
	ec_receive_processdata(EC_TIMEOUTRET);

	// Calculate the elapsed time in seconds
	double elapsedTime = static_cast<double>(currentTime.QuadPart - prevTime.QuadPart) / frequency.QuadPart;

	// Update the previous time
	prevTime = currentTime;

	cnts[(int)(elapsedTime * 1000)]++;

	// Your code to be executed in the timer callback
	// std::cout << elapsedTime * 1000 << std::endl;
}

double get_microseconds()
{
#ifdef _WIN32
	LARGE_INTEGER frequency;
	LARGE_INTEGER start_time;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start_time);

	double microseconds = (double)start_time.QuadPart * 1000000.0 / (double)frequency.QuadPart;

	return microseconds;
#else
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);

	// Calculate the time in microseconds
	long long microseconds = (long long)currentTime.tv_sec * 1000000 + (long long)currentTime.tv_usec;

	return microseconds;
#endif
}

int main(int argc, char* argv[]) {
	char* ifname = argv[1];
	// Initialize the frequency and prevTime variables
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&prevTime);

	TIMECAPS tc;
	timeGetDevCaps(&tc, sizeof(TIMECAPS));
	timeBeginPeriod(tc.wPeriodMin); // Request high-resolution timer

	int i, chk;

	if (ec_init(ifname)) {
		printf("ec_init on %s succeeded.\n", ifname);

		if (ec_config(FALSE, &IOmap) > 0) {
			printf("%d slaves found and configured.\n", ec_slavecount);

			printf("Slaves mapped, state to SAFE_OP.\n");
			/* wait for all slaves to reach SAFE_OP state */
			ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);

			ec_slave[0].state = EC_STATE_OPERATIONAL;
			/* send one valid process data to make outputs in slaves happy*/
			ec_send_processdata();
			ec_receive_processdata(EC_TIMEOUTRET);
			/* request OP state for all slaves */
			ec_writestate(0);
			chk = 200;
			/* wait for all slaves to reach OP state */
			do
			{
				ec_send_processdata();
				ec_receive_processdata(EC_TIMEOUTRET);
				ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
			} while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));

			if (ec_slave[0].state == EC_STATE_OPERATIONAL)
			{
				printf("Operational state reached for all slaves.\n");

				timeSetEvent(1, 0, TimerCallback, 0, TIME_PERIODIC); // 1ms interval
			}
		}
	}

	std::cout << "Press Enter to exit..." << std::endl;
	std::cin.get(); // Wait for user input to exit

	timeEndPeriod(tc.wPeriodMin); // Release high-resolution timer

	return 0;
}
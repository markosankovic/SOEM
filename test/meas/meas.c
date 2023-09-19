#include <stdio.h>

#include "ethercat.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

char IOmap[4096];
#define LOOPCNT 10000
double tstart = 0;
double times[LOOPCNT];

double get_microseconds() {
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

int write_to_file() {
	FILE* file = fopen("times.csv", "w");

	if (file == NULL) {
		printf("Error opening the file.\n");
		return 1;
	}

	for (int i = 0; i < LOOPCNT; i++) {
		fprintf(file, "%f\n", times[i] - tstart);
	}

	fclose(file);

	return 0;
}

void meas(char* ifname)
{
	int i, chk;

	if (ec_init(ifname))
	{
		printf("ec_init on %s succeeded.\n", ifname);

		if (ec_config(FALSE, &IOmap) > 0)
		{
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

				tstart = get_microseconds();
				for (i = 0; i < LOOPCNT; i++) {
					ec_send_processdata();
					ec_receive_processdata(EC_TIMEOUTRET);
					times[i] = get_microseconds();
				}
			}
			else
			{
				printf("Not all slaves reached operational state.\n");
				ec_readstate();
				for (i = 1; i <= ec_slavecount; i++)
				{
					if (ec_slave[i].state != EC_STATE_OPERATIONAL)
					{
						printf("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n",
							i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
					}
				}
			}
		}
	}
}

int main(int argc, char* argv[])
{
	ec_adaptert* adapter = NULL;
	printf("SOMANET SOEM Measurement\n");

	if (argc > 1)
	{
		meas(argv[1]);
		write_to_file();
	}

	return 0;
}

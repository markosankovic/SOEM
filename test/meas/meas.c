#include <stdio.h>
#include <stdlib.h>

#include "ethercat.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

char IOmap[4096];
int iter_cnt = 0;
double *durations;
double t1 = 0;
double t2 = 0;

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

int write_to_file()
{
	FILE *file = fopen("durations.csv", "w");

	if (file == NULL)
	{
		printf("Error opening the file.\n");
		return 1;
	}

	for (int i = 0; i < iter_cnt; i++)
	{
		fprintf(file, "%f,%f\n", durations[i * 2 + 0] / 1000, durations[i * 2 + 1] / 1000);
	}

	fclose(file);

	return 0;
}

void meas(char *ifname)
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

				int wkc = 0;

				for (i = 0; i < iter_cnt; i++)
				{
					t1 = get_microseconds();
					ec_send_processdata();
					t2 = get_microseconds();
					durations[i * 2 + 0] = t2 - t1;
					t1 = t2;
					wkc = ec_receive_processdata(EC_TIMEOUTRET);
					t2 = get_microseconds();
					durations[i * 2 + 1] = t2 - t1;
				}
				printf("Last wkc %d\n", wkc);
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

int main(int argc, char *argv[])
{
	printf("SOMANET SOEM Measurement\n");

	if (argc > 2)
	{
		iter_cnt = atoi(argv[2]);
		int size = iter_cnt * 2;
		durations = (double *)malloc(size * sizeof(double));

		if (durations == NULL)
		{
			printf("ERROR: Failed to allocated array of size %d\n", size);
			return -1;
		}

		meas(argv[1]);

		write_to_file();

		free(durations);
	}

	return 0;
}

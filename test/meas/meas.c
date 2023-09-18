#include <stdio.h>

#include "ethercat.h"

char IOmap[4096];

void meas(char *ifname)
{
  if (ec_init(ifname))
  {
    printf("ec_init on %s succeeded.\n", ifname);

    if (ec_config(FALSE, &IOmap) > 0)
    {
      printf("%d slaves found and configured.\n", ec_slavecount);

      /* wait for all slaves to reach SAFE_OP state */
      ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);

      ec_slave[0].state = EC_STATE_OPERATIONAL;
      /* send one valid process data to make outputs in slaves happy*/
      ec_send_processdata();
      ec_receive_processdata(EC_TIMEOUTRET);

      ec_writestate(0);
    }
  }
}

int main(int argc, char *argv[])
{
  ec_adaptert *adapter = NULL;
  printf("SOMANET SOEM Measurement\n");

  if (argc > 1)
  {
    meas(argv[1]);
  }

  return 0;
}

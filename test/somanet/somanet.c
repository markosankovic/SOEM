#include <stdio.h>
#include <string.h>
#include "mongoose.h"
#include "ethercat.h"

static const char *s_http_addr = "http://localhost:8000"; // HTTP port
static const char *s_root_dir = "web_root";

char IOmap[4096];
ec_adaptert *adapter = NULL;

void init(char *ifname)
{
  int i, cnt;

  if (ec_init(ifname))
  {
    printf("ec_init on %s succeeded.\n", ifname);

    if (ec_config(FALSE, &IOmap) > 0)
    {
      ec_configdc();
      while (EcatError)
      {
        printf("%s", ec_elist2string());
      }
      printf("%d slaves found and configured.\n", ec_slavecount);

      ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * ec_slavecount);

      if (ec_slave[0].state != EC_STATE_SAFE_OP)
      {
        printf("Not all slaves reached safe operational state.\n");
        ec_readstate();
        for (i = 1; i <= ec_slavecount; i++)
        {
          if (ec_slave[i].state != EC_STATE_SAFE_OP)
          {
            printf("Slave %d State=%2x StatusCode=%4x : %s\n",
                   i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
          }
        }
      }

      ec_readstate();
      for (cnt = 1; cnt <= ec_slavecount; cnt++)
      {
        printf("\nSlave:%d\n Name:%s\n Output size: %dbits\n Input size: %dbits\n State: %d\n Delay: %d[ns]\n Has DC: %d\n",
               cnt, ec_slave[cnt].name, ec_slave[cnt].Obits, ec_slave[cnt].Ibits,
               ec_slave[cnt].state, ec_slave[cnt].pdelay, ec_slave[cnt].hasdc);

        printf(" Configured address: %4.4x\n", ec_slave[cnt].configadr);
        printf(" Man: %8.8x ID: %8.8x Rev: %8.8x\n", (int)ec_slave[cnt].eep_man, (int)ec_slave[cnt].eep_id, (int)ec_slave[cnt].eep_rev);
      }
    }
  }
}

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
  if (ev == MG_EV_HTTP_MSG)
  {
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;

    printf("\n_________________________________________________________________________\n");
    printf("%s\n\n", hm->method.ptr);

    if (mg_http_match_uri(hm, "/api/getAdapters"))
    {
      char names[EC_MAXLEN_ADAPTERNAME * 10];
      int k = 0;
      names[k++] = '[';
      ec_adaptert *item = adapter;
      while (item != NULL)
      {
        char *name = mg_mprintf("\"%s\"", item->name);
        printf("%s\r\n", name);
        for (unsigned int i = 0; i < strlen(name); i++)
        {
          names[k++] = name[i] == '\\' ? '/' : name[i];
        }
        item = item->next;
        if (item != NULL)
        {
          names[k++] = ',';
        }
      }
      names[k++] = ']';
      names[k++] = '\0';
      mg_http_reply(c, 200,
                    "Access-Control-Allow-Origin: *\r\n"
                    "Content-Type: application/json\r\n",
                    "%s\n", names);
    }
    else if (mg_http_match_uri(hm, "/api/connect"))
    {
      struct mg_str json = hm->body;
      int result = -1;
      char *ifname = mg_json_get_str(json, "$.adapter");
      if (ifname != NULL)
      {
        printf("Connect to %s\r\n", ifname);
        init(ifname);
        result = 0;
      }
      mg_http_reply(c, 200,
                    "Access-Control-Allow-Headers: *\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Content-Type: application/json\r\n",
                    "{%Q:%d}\n", "slaveCount", ec_slavecount);
    }
    else if (mg_http_match_uri(hm, "/api/getSlaveInfo*"))
    {
      char param[255];
      mg_http_get_var(&hm->query, "slave", param, 255);
      int slave = atoi(param);
      mg_http_reply(c, 200,
                    "Access-Control-Allow-Headers: *\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Content-Type: application/json\r\n",
                    "{%Q:\"0x%x\",%Q:%Q,%Q:%d,%Q:%d,%Q:%d,%Q:\"0x%x\",%Q:\"0x%x\",%Q:\"0x%x\"}\n",
                    "configuredAddress", ec_slave[slave].configadr,
                    "name", ec_slave[slave].name,
                    "outputSize", ec_slave[slave].Obits,
                    "inputSize", ec_slave[slave].Ibits,
                    "state", ec_slave[slave].state,
                    "manufacturer", (int)ec_slave[slave].eep_man,
                    "id", (int)ec_slave[slave].eep_id,
                    "revision", (int)ec_slave[slave].eep_rev);
    }
    else if (mg_http_match_uri(hm, "/api/readSDO"))
    {
      mg_http_reply(c, 200,
                    "Access-Control-Allow-Origin: *\r\n"
                    "Content-Type: application/json\r\n",
                    "{%Q:%d}\n", "result", 0);
    }
    else if (mg_http_match_uri(hm, "/api/writeSDO"))
    {
      mg_http_reply(c, 200,
                    "Access-Control-Allow-Origin: *\r\n"
                    "Content-Type: application/json\r\n",
                    "{%Q:%d}\n", "result", 0);
    }
    else
    {
      struct mg_http_serve_opts opts = {.root_dir = s_root_dir};
      mg_http_serve_dir(c, ev_data, &opts);
    }
  }
  (void)fn_data;
}

int main(int argc, char *argv[])
{
  adapter = ec_find_adapters(); // Create linked list of available adapters

  struct mg_mgr mgr;                           // Event manager
  mg_log_set(MG_LL_DEBUG);                     // Set log level
  mg_mgr_init(&mgr);                           // Initialize event manager
  mg_http_listen(&mgr, s_http_addr, fn, NULL); // Create HTTP listener
  for (;;)
    mg_mgr_poll(&mgr, 1000);
  mg_mgr_free(&mgr);

  ec_free_adapters(adapter);

  return 0;
}

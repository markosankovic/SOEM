#include <stdio.h>
#include <string.h>
#include "mongoose.h"
#include "ethercat.h"

static const char *s_http_addr = "http://localhost:8000"; // HTTP port
static const char *s_root_dir = "web_root";

char IOmap[4096];
ec_adaptert *adapter = NULL;
char usdo[128];

char *SDO2string(uint16 slave, uint16 index, uint8 subidx, uint16 dtype)
{
  int l = sizeof(usdo) - 1, i;
  uint8 *u8;
  int8 *i8;
  uint16 *u16;
  int16 *i16;
  uint32 *u32;
  int32 *i32;
  uint64 *u64;
  int64 *i64;
  float *sr;
  double *dr;
  char es[32];

  memset(&usdo, 0, 128);
  ec_SDOread(slave, index, subidx, FALSE, &l, &usdo, EC_TIMEOUTRXM);
  if (EcatError)
  {
    return ec_elist2string();
  }
  else
  {
    static char str[64] = {0};
    switch (dtype)
    {
    case ECT_BOOLEAN:
      u8 = (uint8 *)&usdo[0];
      if (*u8)
        sprintf(str, "TRUE");
      else
        sprintf(str, "FALSE");
      break;
    case ECT_INTEGER8:
      i8 = (int8 *)&usdo[0];
      sprintf(str, "0x%2.2x / %d", *i8, *i8);
      break;
    case ECT_INTEGER16:
      i16 = (int16 *)&usdo[0];
      sprintf(str, "0x%4.4x / %d", *i16, *i16);
      break;
    case ECT_INTEGER32:
    case ECT_INTEGER24:
      i32 = (int32 *)&usdo[0];
      sprintf(str, "0x%8.8x / %d", *i32, *i32);
      break;
    case ECT_INTEGER64:
      i64 = (int64 *)&usdo[0];
      sprintf(str, "0x%16.16" PRIx64 " / %" PRId64, *i64, *i64);
      break;
    case ECT_UNSIGNED8:
      u8 = (uint8 *)&usdo[0];
      sprintf(str, "0x%2.2x / %u", *u8, *u8);
      break;
    case ECT_UNSIGNED16:
      u16 = (uint16 *)&usdo[0];
      sprintf(str, "0x%4.4x / %u", *u16, *u16);
      break;
    case ECT_UNSIGNED32:
    case ECT_UNSIGNED24:
      u32 = (uint32 *)&usdo[0];
      sprintf(str, "0x%8.8x / %u", *u32, *u32);
      break;
    case ECT_UNSIGNED64:
      u64 = (uint64 *)&usdo[0];
      sprintf(str, "0x%16.16" PRIx64 " / %" PRIu64, *u64, *u64);
      break;
    case ECT_REAL32:
      sr = (float *)&usdo[0];
      sprintf(str, "%f", *sr);
      break;
    case ECT_REAL64:
      dr = (double *)&usdo[0];
      sprintf(str, "%f", *dr);
      break;
    case ECT_BIT1:
    case ECT_BIT2:
    case ECT_BIT3:
    case ECT_BIT4:
    case ECT_BIT5:
    case ECT_BIT6:
    case ECT_BIT7:
    case ECT_BIT8:
      u8 = (uint8 *)&usdo[0];
      sprintf(str, "0x%x / %u", *u8, *u8);
      break;
    case ECT_VISIBLE_STRING:
      strcpy(str, "\"");
      strcat(str, usdo);
      strcat(str, "\"");
      break;
    case ECT_OCTET_STRING:
      str[0] = 0x00;
      for (i = 0; i < l; i++)
      {
        sprintf(es, "0x%2.2x ", usdo[i]);
        strcat(str, es);
      }
      break;
    default:
      sprintf(str, "Unknown type");
    }
    return str;
  }
}

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
    else if (mg_http_match_uri(hm, "/api/readSdo"))
    {
      char param[255];
      mg_http_get_var(&hm->query, "slave", param, 255);
      int slave = atoi(param);
      mg_http_get_var(&hm->query, "index", param, 255);
      int index = atoi(param);
      mg_http_get_var(&hm->query, "subindex", param, 255);
      int subindex = atoi(param);
      char *value = SDO2string(slave, index, subindex, ECT_INTEGER32);
      printf("slave: %d, index: %d, subindex: %d, value: %s\n", slave, index, subindex, value);
      mg_http_reply(c, 200,
                    "Access-Control-Allow-Origin: *\r\n"
                    "Content-Type: application/json\r\n",
                    "{%Q:\"%s\"}\n", "value", value);
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

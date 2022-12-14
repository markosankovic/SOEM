#include <stdio.h>
#include <string.h>
#include "mongoose.h"
#include "ethercat.h"

static const char *s_http_addr = "http://127.0.0.1:8000"; // HTTP port
static const char *s_root_dir = "web_root";

ec_adaptert *adapter = NULL;

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
  if (ev == MG_EV_HTTP_MSG)
  {
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;

    printf("\n_________________________________________________________________________\n");
    printf("%s", hm->method.ptr);

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
      char *id = mg_json_get_str(json, "$.adapter");
      if (id != NULL)
      {
        printf("Connect to %s\r\n", id);
        result = 0;
      }
      mg_http_reply(c, 200,
                    "Access-Control-Allow-Headers: *\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Content-Type: application/json\r\n",
                    "{%Q:%d}\n", "result", result);
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

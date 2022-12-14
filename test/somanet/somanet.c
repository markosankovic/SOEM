#include <stdio.h>
#include <string.h>
#include "mongoose.h"
#include "ethercat.h"

static const char *s_http_addr = "http://localhost:8000"; // HTTP port
static const char *s_root_dir = "web_root";

ec_adaptert *adapter = NULL;

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    if (mg_http_match_uri(hm, "/api/getAdapters")) {
      char names[EC_MAXLEN_ADAPTERNAME * 10];
      int k = 0;
      names[k++] = '[';
      ec_adaptert *item = adapter;
      while (item != NULL) {
        char *name = mg_mprintf("\"%s\"", item->name);
        printf("%s\n", name);
        for (unsigned int i = 0; i < strlen(name); i++) {
          names[k++] = name[i] == '\\' ? '/' : name[i]; // required for JSON, changing path delimiter is okay in Windows
        }
        item = item->next;
        if (item != NULL) {
          names[k++] = ',';
        }
      }
      names[k++] = ']';
      names[k++] = '\0';
      mg_http_reply(c, 200, "Content-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\n", "%s\n", names);
    } else {
      struct mg_http_serve_opts opts = {.root_dir = s_root_dir};
      mg_http_serve_dir(c, ev_data, &opts);
    }
  }
  (void) fn_data;
}

int main(int argc, char *argv[]) {
  adapter = ec_find_adapters();

  struct mg_mgr mgr; // Event manager
  mg_log_set(MG_LL_DEBUG); // Set log level
  mg_mgr_init(&mgr); // Initialize event manager
  mg_http_listen(&mgr, s_http_addr, fn, NULL); // Create HTTP listener
  for (;;) mg_mgr_poll(&mgr, 1000);
  mg_mgr_free(&mgr);

  return 0;
}

/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include <cstdlib>
#include <string>
#include <cstring>
#include <map>
#include <utility>
#include <iostream>
#include <sstream>
#include "trace.h"
#include "importfunctor.h"
#include <cstdio>
#include "external/mongoose.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static const char *s_http_port = "10005";
static struct mg_serve_http_opts s_http_server_opts;

Trace * trace = NULL;
json j;
bool logging = false;
bool server_logging = true;

static void handle_data_call(struct mg_connection *nc, struct http_message *hm) {
  const std::string sep = "\r\n";

  std::stringstream ss;
  ss << "HTTP/1.1 200 OK"             << sep
    << "Content-Type: application/json" << sep
    << "Access-Control-Allow-Origin: *" << sep
    << "Content-Length: %d"             << sep << sep
    << "%s";

  json j = json::parse(std::string(hm->body.p, hm->body.len));

  /* Get command variables */
  std::string cmd;
  cmd = j["command"];

  std::cout << "And the dump is... " << j.dump().c_str() << std::endl;

  /* Check for trace info */
  if (cmd.compare("time") == 0)
  {
    std::string start, stop, entity_start, entities, task,
                task_time, hover;
    long width;
    start = j["start"];
    stop = j["stop"];
    entity_start = j["entity_start"];
    entities = j["entities"];
    width = j["width"];
    task = j["focus_task"];
    task_time = j["focus_time"];
    hover = j["hover_task"];
    
    if (logging || server_logging) {
      std::cout << "Request for start/stop (" << start << ", " << stop << ") and ";
      std::cout << "entity_start/entities/width " << entity_start << ", " << entities;
      std::cout << ", " << width << " and task/time " << task << "/" << task_time;
      std::cout << ", hover: " << hover << std::endl;

    }
    j["traceinfo"] = trace->timeToJSON(std::stoull(start.c_str()),
                                       std::stoull(stop.c_str()), 
                                       std::stoull(entity_start.c_str()),
                                       std::stoull(entities.c_str()),
                                       width,
                                       std::stoull(task.c_str()),
                                       std::stoull(task_time.c_str()),
                                       std::stoull(hover.c_str()),
                                       logging);
  }
  else if (cmd.compare("load") == 0)
  {
    long width;
    width = j["width"];
    j["traceinfo"] = trace->initJSON(width, logging);
    if (server_logging) {
      std::cout << "initJSON called." << std::endl;
    }
  }
  else if (cmd.compare("overview") == 0)
  {
    long width;
    width = j["width"];
    j["traceinfo"] = trace->timeOverview(width, logging);
    if (server_logging) {
      std::cout << "overview called." << std::endl;
    }
  }
  else
  {
    j["debug"] = 0;
    if (server_logging) {
      std::cout << "NO COMMAND called." << std::endl;
    }
  }

  mg_printf(nc, ss.str().c_str(), (int) j.dump().size(), j.dump().c_str());
}


static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  struct http_message *hm = (struct http_message *) ev_data;

  switch (ev) {
    case MG_EV_HTTP_REQUEST:
      std::cout << "HM->URI.P IS: " << hm->uri.p << std::endl;
      if (mg_vcmp(&hm->uri, "/playground/kisaacs/traveler/data") == 0
	|| mg_vcmp(&hm->uri, "/data") == 0
        || mg_vcmp(&hm->uri, "https://hdc.cs.arizona.edu/playground/kisaacs/traveler/data") == 0) {
        std::cout << "Serving DATA CONTENT." << std::endl;
        std::cout << "DATA CONTENT HM->body is " << hm->body.p << std::endl;
        handle_data_call(nc, hm); // Handle RESTful call 
      } else if (mg_vcmp(&hm->uri, "/printcontent") == 0) {
        char buf[100] = {0};
        memcpy(buf, hm->body.p,
               sizeof(buf) - 1 < hm->body.len ? sizeof(buf) - 1 : hm->body.len);
        printf("%s\n", buf);
      } else {
        std::cout << "HM->MESSAGE is " << hm->message.p << std::endl;
        mg_serve_http(nc, hm, s_http_server_opts); // Serve static content 
        std::cout << "Serving STATIC: " << std::endl;
      }
      break;
    default:
      break;
  }
}


static void setTrace(std::string dataFileName) {
    trace = NULL;
    if (dataFileName.length() == 0) {
        std::cout << "No trace file given." << std::endl;
    }

    ImportFunctor * importWorker = new ImportFunctor();

    if (dataFileName.compare(dataFileName.length() - 4, 3, "otf"))
    {
        trace = importWorker->doImportOTF(dataFileName, logging);
    }
    else if (dataFileName.compare(dataFileName.length() - 5, 4, "otf2"))
    {
        trace = importWorker->doImportOTF2(dataFileName, logging);
    }
    else
    {
        std::cout << "Unrecognized trace format!" << std::endl;
    }
    delete importWorker;
}

int main(int argc, char *argv[]) {
  struct mg_mgr mgr;
  struct mg_connection *nc;
  struct mg_bind_opts bind_opts;
  int i;
  char *cp;
  const char *err_str;
#if MG_ENABLE_SSL
  const char *ssl_cert = NULL;
#endif

  mg_mgr_init(&mgr, NULL);

  /* Use current binary directory as document root */

  if (argc > 0 && ((cp = strrchr(argv[0], DIRSEP)) != NULL)) {
    *cp = '\0';
    s_http_server_opts.document_root = argv[0];
  }

  std::string filename = "";
  /* Process command line options to customize HTTP server */
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-D") == 0 && i + 1 < argc) {
      mgr.hexdump_file = argv[++i];
    } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
      s_http_server_opts.document_root = argv[++i];
    } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
      s_http_port = argv[++i];
    } else if (strcmp(argv[i], "-a") == 0 && i + 1 < argc) {
      s_http_server_opts.auth_domain = argv[++i];
#if MG_ENABLE_JAVASCRIPT
    } else if (strcmp(argv[i], "-j") == 0 && i + 1 < argc) {
      const char *init_file = argv[++i];
      mg_enable_javascript(&mgr, v7_create(), init_file);
#endif
    } else if (strcmp(argv[i], "-P") == 0 && i + 1 < argc) {
      s_http_server_opts.global_auth_file = argv[++i];
    } else if (strcmp(argv[i], "-A") == 0 && i + 1 < argc) {
      s_http_server_opts.per_directory_auth_file = argv[++i];
    } else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
      s_http_server_opts.url_rewrites = argv[++i];
#if MG_ENABLE_HTTP_CGI
    } else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
      s_http_server_opts.cgi_interpreter = argv[++i];
#endif
#if MG_ENABLE_SSL
    } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
      ssl_cert = argv[++i];
#endif
    } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
        filename = argv[++i];
        setTrace(filename);
    } else if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
        logging = true;
    } else {
      fprintf(stderr, "Unknown option: [%s]\n", argv[i]);
      exit(1);
    }
  }

  /* Set HTTP server options */

  memset(&bind_opts, 0, sizeof(bind_opts));
  bind_opts.error_string = &err_str;
#if MG_ENABLE_SSL
  if (ssl_cert != NULL) {
    bind_opts.ssl_cert = ssl_cert;
  }
#endif
  nc = mg_bind_opt(&mgr, s_http_port, ev_handler, bind_opts);
  if (nc == NULL) {
    fprintf(stderr, "Error starting server on port %s: %s\n", s_http_port,
            *bind_opts.error_string);
    exit(1);
  }

  mg_set_protocol_http_websocket(nc);
  s_http_server_opts.enable_directory_listing = "yes";

  printf("Starting RESTful server on port %s, serving %s\n", s_http_port,
         s_http_server_opts.document_root);
  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return 0;
}

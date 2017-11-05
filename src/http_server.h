#ifndef C_HTTP_SERVER_HTTP_SERVER_H
#define C_HTTP_SERVER_HTTP_SERVER_H

#include "http_spec.h"

// Declaration of the functions used in the program

void set_up(HE *server, int port);

void shut_down(HE *server);

void loop_accepting(HE *server);

void process_client(HE *server, HE *client);

AWL *read_until_crlf(HE *client, int *result);

HREQ *construct_request(AWL *request_line);

void get_parameters_if_present(HREQ *request);

void add_header_to_response(HRES *response, AWL *request_line);

HP *find_parameter(HP *first, AWL *name);

void add_header_to_request(HREQ *request, AWL *request_line);

HH *find_header(HH *first, AWL *name);

void get_body_if_present(HE *client, HREQ *request);

void write_response(HE *client, HRES *response);

void free_request(HREQ *request);

void free_response(HRES *response);

void free_parameter(HP *parameter);

void free_header(HH *header);

HRES *construct_error(HE *server, HE *client, HREQ *request);

void log_event(HE *server, HE *client, HREQ *request, HRES *response);

#endif //C_HTTP_SERVER_HTTP_SERVER_H

#include "http_post.h"

// Creating the POST response
HRES *construct_post_response(HE *server, HE *client, HREQ *request) {
    HRES *res = malloc(sizeof(HRES));
    alloc_awl(res->version, request->version->length)
    strncpy(res->version->array, request->version->array, res->version->length);
    alloc_awl(res->status_code, OK_CODE_LENGTH)
    strncpy(res->status_code->array, OK_CODE, OK_CODE_LENGTH);
    alloc_awl(res->status_message, OK_MESSAGE_LENGTH)
    strncpy(res->status_message->array, OK_MESSAGE, OK_MESSAGE_LENGTH);
    res->header = NULL;
    res->body = NULL;

    char html_start[] = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"
            "<title>PA2_SERVER</title>"
            "</head>"
            "<body>"
            "<p>";
    size_t html_start_length = strlen(html_start);
    alloc_awl(res->body, html_start_length);
    strncpy(res->body->array, html_start, html_start_length);

    char char_localhost[] = "http://localhost:";
    size_t char_localhost_length = strlen(char_localhost);
    append_to_awl(res->body, char_localhost, char_localhost_length);

    char char_server_port[6];
    memset(&char_server_port, 0, sizeof(char) * 6);
    sprintf(char_server_port, "%i", ntohs(server->sock_addr.sin_port));
    size_t char_server_port_length = strlen(char_server_port);
    append_to_awl(res->body, char_server_port, char_server_port_length);

    append_to_awl(res->body, request->url->array, request->url->length);

    append_to_awl(res->body, " ", 1);

    char char_client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client->sock_addr.sin_addr), char_client_ip, INET_ADDRSTRLEN);
    size_t char_client_ip_length = strlen(char_client_ip);
    append_to_awl(res->body, char_client_ip, char_client_ip_length);

    append_to_awl(res->body, ":", 1);

    char char_client_port[6];
    memset(&char_client_port, 0, sizeof(char) * 6);
    sprintf(char_client_port, "%i", ntohs(client->sock_addr.sin_port));
    size_t char_client_port_length = strlen(char_client_port);
    append_to_awl(res->body, char_client_port, char_client_port_length);

    char html_middle[] = "</p><p>";
    size_t html_middle_length = strlen(html_middle);
    append_to_awl(res->body, html_middle, html_middle_length);

    // Append the request body to the response body
    append_to_awl(res->body, request->body->array, request->body->length);

    char html_end[] = "</p>"
            "</body>"
            "</html>";
    size_t html_end_length = strlen(html_end);
    append_to_awl(res->body, html_end, html_end_length);

    HH *cl = malloc(sizeof(HH));
    cl->next = NULL;
    alloc_awl(cl->name, CL_LENGTH)
    strncpy(cl->name->array, CL, CL_LENGTH);
    char char_cl[64];
    memset(&char_cl, 0, sizeof(char) * 64);
    sprintf(char_cl, "%i", res->body->length);
    size_t char_cl_length = strlen(char_cl);
    alloc_awl(cl->value, char_cl_length)
    strncpy(cl->value->array, char_cl, char_cl_length);

    HH *ct = malloc(sizeof(HH));
    ct->next = cl;
    alloc_awl(ct->name, CT_LENGTH)
    strncpy(ct->name->array, CT, CT_LENGTH);
    alloc_awl(ct->value, TEXT_HTML_LENGTH)
    strncpy(ct->value->array, TEXT_HTML, TEXT_HTML_LENGTH);

    res->header = ct;
    return res;
}

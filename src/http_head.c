#include "http_head.h"
#include "http_get.h"

// Creating the HEAD response
HRES *construct_head_response(HE *server, HE *client, HREQ *request) {
    // Head response is the same as GET request just without the body
    // so the server calls the GET request and deletes the body
    HRES *res = construct_get_response(server, client, request);
    dealloc_awl(res->body)
    res->body = NULL;
    return res;
}

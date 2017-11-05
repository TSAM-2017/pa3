#include "http_get.h"
#include "http_server.h"

// Creating the GET response
HRES *construct_get_response(HE *server, HE *client, HREQ *request) {
    // Allocate memory for the response
    HRES *res = malloc(sizeof(HRES));
    alloc_awl(res->version, request->version->length)
    strncpy(res->version->array, request->version->array, res->version->length);
    // Setting the responses HTTP code
    alloc_awl(res->status_code, OK_CODE_LENGTH)
    strncpy(res->status_code->array, OK_CODE, OK_CODE_LENGTH);
    // Setting the responses HTTP message
    alloc_awl(res->status_message, OK_MESSAGE_LENGTH)
    strncpy(res->status_message->array, OK_MESSAGE, OK_MESSAGE_LENGTH);
    res->header = NULL;
    res->body = NULL;
    // Copy the start of the HTML to the response body
    char html_start[] = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"
            "<title>PA3_SERVER</title>"
            "</head>"
            "<body";
    size_t html_start_length = strlen(html_start);
    alloc_awl(res->body, html_start_length);
    strncpy(res->body->array, html_start, html_start_length);

    // Check if the client is requesting the color page
    char color_uri[] = "/color";
    size_t color_uri_length = strlen(color_uri);
    size_t color_uri_min = min(request->url->length, color_uri_length);
    if (color_uri_min >= color_uri_length && strncmp(request->url->array, color_uri, (size_t) color_uri_min) == 0) {
        char bg_char[] = "bg";
        size_t bg_char_length = strlen(bg_char);
        AWL *bg_awl;
        alloc_awl(bg_awl, bg_char_length)
        strncpy(bg_awl->array, bg_char, bg_char_length);
        // Find the bg parameter
        HP *color_parameter = find_parameter(request->parameter, bg_awl);
        dealloc_awl(bg_awl)

        if (color_parameter != NULL) {
            // bg parameter found, set style to the HTML
            char style_char[] = " style=\"background-color:";
            size_t style_char_length = strlen(style_char);
            append_to_awl(res->body, style_char, style_char_length)

            append_to_awl(res->body, color_parameter->value->array, color_parameter->value->length)

            char quote_char[] = "\"";
            size_t quote_char_length = strlen(quote_char);
            append_to_awl(res->body, quote_char, quote_char_length)
            // Return a cookie header to the client
            AWL *set_cookie;
            char set_cookie_char[] = "Set-Cookie: bg=";
            size_t set_cookie_char_length = strlen(set_cookie_char);
            alloc_awl(set_cookie, set_cookie_char_length)
            strncpy(set_cookie->array, set_cookie_char, set_cookie_char_length);
            append_to_awl(set_cookie, color_parameter->value->array, color_parameter->value->length)
            add_header_to_response(res, set_cookie);
        } else {
            // No parameter found, try to find the cookie
            char cookie_char[] = "Cookie";
            size_t cookie_char_length = strlen(cookie_char);
            AWL *cookie_awl;
            alloc_awl(cookie_awl, cookie_char_length)
            strncpy(cookie_awl->array, cookie_char, cookie_char_length);
            HH *color_cookie = find_header(request->header, cookie_awl);
            dealloc_awl(cookie_awl)
            cookie_awl = NULL;
            // If the cookie header was found
            if (color_cookie != NULL) {
                int i = 0, j;
                int equal = 0;
                int block = 0;
                // Search for the = sign that defines the end of the name of the cookie that the server specified
                while (i < color_cookie->value->length && color_cookie->value->array[i] != EQ) {
                    ++equal;
                    ++i;
                }
                ++i;
                // Search for the ; sign that defines the end of the value of the cookie that the server specified
                while (i < color_cookie->value->length && color_cookie->value->array[i] != SC) {
                    ++block;
                    ++i;
                }
                // If the name and value have valid lengths, retrieve the cookie value
                if (equal != 0 && block != 0) {
                    // Allocate memory for the cookie value
                    alloc_awl(cookie_awl, block)

                    int size = block;
                    for (j = 0; j < block; ++j) {
                        // Copy the cookie value
                        cookie_awl->array[j] = color_cookie->value->array[i - size + j];
                    }
                }
                // If the cookie value was found, use it to set the background style
                if (cookie_awl != NULL) {
                    char style_char[] = " style=\"background-color:";
                    size_t style_char_length = strlen(style_char);
                    append_to_awl(res->body, style_char, style_char_length)

                    append_to_awl(res->body, cookie_awl->array, cookie_awl->length)

                    char quote_char[] = "\"";
                    size_t quote_char_length = strlen(quote_char);
                    append_to_awl(res->body, quote_char, quote_char_length)

                    dealloc_awl(cookie_awl)
                }
            }
        }
    }

    // Append the closing > for the body, and open a paragraph
    char p[] = "><p>";
    size_t p_length = strlen(p);
    append_to_awl(res->body, p, p_length)

    // Append the localhost string
    char char_localhost[] = "http://localhost:";
    size_t char_localhost_length = strlen(char_localhost);
    append_to_awl(res->body, char_localhost, char_localhost_length)

    // Append the server port
    char char_server_port[6];
    memset(&char_server_port, 0, sizeof(char) * 6);
    sprintf(char_server_port, "%i", ntohs(server->sock_addr.sin_port));
    size_t char_server_port_length = strlen(char_server_port);
    append_to_awl(res->body, char_server_port, char_server_port_length)

    // Append the request URL
    append_to_awl(res->body, request->url->array, request->url->length)

    // Append the space
    append_to_awl(res->body, " ", 1)

    // Append the client IP
    char char_client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client->sock_addr.sin_addr), char_client_ip, INET_ADDRSTRLEN);
    size_t char_client_ip_length = strlen(char_client_ip);
    append_to_awl(res->body, char_client_ip, char_client_ip_length)

    // Append a colon
    append_to_awl(res->body, ":", 1)

    // Append client port
    char char_client_port[6];
    memset(&char_client_port, 0, sizeof(char) * 6);
    sprintf(char_client_port, "%i", ntohs(client->sock_addr.sin_port));
    size_t char_client_port_length = strlen(char_client_port);
    append_to_awl(res->body, char_client_port, char_client_port_length)

    // Checking test URI
    char test_uri[] = "/test";
    size_t test_uri_length = strlen(test_uri);
    size_t test_uri_min = min(request->url->length, test_uri_length);
    // If the requested page is test OR ENABLE_PARAMETERS_FOR_ALL == 1
    // the server will append all the query parameters to the response body
    if ((ENABLE_PARAMETERS_FOR_ALL ||
         (test_uri_min >= test_uri_length && strncmp(request->url->array, test_uri, (size_t) test_uri_min) == 0)) &&
        request->parameter != NULL) {
        HP *parameter = request->parameter;
        char start_p[] = "</p><p>";
        size_t start_p_length = strlen(start_p);
        // Loop until the server find a NULL parameter
        while (parameter != NULL) {
            // Append a close and open paragraph
            append_to_awl(res->body, start_p, start_p_length)
            // Append parameter name
            append_to_awl(res->body, parameter->name->array, parameter->name->length)
            // Append the equals sign
            append_to_awl(res->body, "=", 1)
            // Append the parameter value
            append_to_awl(res->body, parameter->value->array, parameter->value->length)

            parameter = parameter->next;
        }
    }

    // Append the closing HTML
    char html_end[] = "</p>"
            "</body>"
            "</html>";
    size_t html_end_length = strlen(html_end);
    append_to_awl(res->body, html_end, html_end_length)

    // Create the content length header
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

    // Create the content type header
    HH *ct = malloc(sizeof(HH));
    ct->next = cl;
    alloc_awl(ct->name, CT_LENGTH)
    strncpy(ct->name->array, CT, CT_LENGTH);
    alloc_awl(ct->value, TEXT_HTML_LENGTH)
    strncpy(ct->value->array, TEXT_HTML, TEXT_HTML_LENGTH);

    // Append the header to the response
    if (res->header == NULL) {
        res->header = ct;
    } else {
        HH *next = res->header;
        while (next->next != NULL) {
            next = next->next;
        }
        next->next = ct;
    }
    return res;
}

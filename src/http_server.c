#include "http_server.h"
#include "http_get.h"
#include "http_head.h"
#include "http_post.h"

// Initializing the server
void set_up(HE *server, int port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        error_out("Failed to open server socket")
    }

    int in = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &in, sizeof(in)) == -1) {
        error_out("Failed to make socket reusable")
    }

    if (ioctl(server_socket, FIONBIO, (char *) &in) == -1) {
        error_out("Failed to make socket non-blocking")
    }

    struct sockaddr_in serv_addr;
    socklen_t server_size = sizeof(serv_addr);
    memset(&serv_addr, 0, server_size);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(server_socket, (struct sockaddr *) &serv_addr, server_size) == -1) {
        error_out("Failed to bind server data");
    }

    if (listen(server_socket, 32) == -1) {
        error_out("Failed to listen server socket")
    }

    // Assigning to the HE structure the server_socket and the sockaddr_in
    server->socket = server_socket;
    server->sock_addr = serv_addr;
}

// Close the server socket
void shut_down(HE *server) {
    if (close(server->socket) == -1) {
        error_out("Failed to close the server socket")
    }
}

// Loop accepting clients
// Poll function
void loop_accepting(HE *server) {
    const int max_size = 33;
    // Initialize the timeout to 3 min.
    // If no activity in 3 min then end program.
    const int timeout = 3 * 60 * 1000;

    // nfds is the number of file descriptors
    nfds_t nfds = 1;

    // Array structure of poll file descriptors
    struct pollfd fds[max_size];
    memset(fds, 0, sizeof(fds));
    fds[0].fd = server->socket;
    fds[0].events = POLLIN;

    // Array of HE structures
    // Hold the socket and the sockaddr_in of the entity
    // to match the length of fds
    HE hes[max_size];
    memset(hes, 0, sizeof(hes));
    hes[0].socket = server->socket;
    hes[0].sock_addr = server->sock_addr;

    int i, j, pr;
    while (1) {
        pr = poll(fds, nfds, timeout);
        if (pr < 0) {
            error_out("Failed to poll")
            // If poll returns -1 an error occurred
        } else if (pr == 0) {
            // if poll return 0 then there was a timeout for the specified ms
            break;
        } else {
            // No error occurred so we can continue
            for (i = 1; i < nfds; ++i) {
                // loop through all the active clients
                if (fds[i].revents == 0) { // if revents = 0 then there were no events for this file descriptor
                    continue;
                } else if (fds[i].revents != POLLIN) { // if revents is not pollin then something went wrong
                    error_out("Client revent != POLLIN")
                } else {
                    // Everything looks good, we can process the client
                    process_client(server, &hes[i]);
                    // TODO: Keep-Alive
                    // Close the client
                    if (close(hes[i].socket) == -1) {
                        error_out("Failed to close the client socket")
                    }
                    fds[i].fd = 0;
                    nfds--;
                    // Compress the array of file descriptors
                    // WE need to do this cause the poll functions
                    // expects an array of continues valid file descriptors
                    if (i < max_size - 1) {
                        for (j = i; j < nfds; ++j) { // Move everything one position to the left
                            fds[j].fd = fds[j + 1].fd;
                            fds[j].events = fds[j + 1].events;
                            hes[j].socket = hes[j + 1].socket;
                            hes[j].sock_addr = hes[j + 1].sock_addr;
                        }
                    }
                }
            }
            // Clear the memory of any invalid position
            for (i = 1; i < max_size; ++i) {
                if (fds[i].fd == 0) {
                    memset(&fds[i], 0, sizeof(struct pollfd));
                    memset(&hes[i], 0, sizeof(HE));
                }
            }
            // If the server has no events, then do nothing
            if (fds[0].revents == 0) {
                continue;
            } else if (fds[0].revents != POLLIN) { // if the servers revents is not pollin then something went wrong
                error_out("Server revent != POLLIN")
            } else {
                while (1) { // Accept clients until server is full or an error happens
                    if (nfds == max_size) { // server is full -stop accepting
                        break;
                    }
                    int client_socket;
                    struct sockaddr_in client_addr;
                    socklen_t client_size = sizeof(client_addr);
                    memset(&client_addr, 0, client_size);
                    client_socket = accept(server->socket, (struct sockaddr *) &client_addr, &client_size);
                    if (client_socket == -1) { // error happens -stop accepting
                        break;
                    }
                    // If we are able to accept the client we save it to the array to process it later
                    fds[nfds].fd = client_socket;
                    fds[nfds].events = POLLIN;
                    hes[nfds].socket = client_socket;
                    hes[nfds].sock_addr = client_addr;
                    ++nfds;
                }
            }
        }
    }
}

// Process the client
void process_client(HE *server, HE *client) {
    int result;
    AWL *request_line = read_until_crlf(client, &result);
    // Construct request based on the request line
    HREQ *request = construct_request(request_line);
    dealloc_awl(request_line)

    get_parameters_if_present(request);

    // Start reading the headers
    while (result > 0) {
        request_line = read_until_crlf(client, &result);
        if (request_line->length == 0) { // end of the headers section
            dealloc_awl(request_line)
            break;
        }
        // Line is a header -call a function to process it
        add_header_to_request(request, request_line);
        dealloc_awl(request_line)
    }

    // Check if we can read the body of the request
    get_body_if_present(client, request);

    HRES *response = NULL;
    // Checking which request the server is getting
    int min = min(request->method->length, GET_LENGTH);
    if (strncmp(request->method->array, GET, (size_t) min) == 0) {
        response = construct_get_response(server, client, request);
    } else {
        min = min(request->method->length, HEAD_LENGTH);
        if (strncmp(request->method->array, HEAD, (size_t) min) == 0) {
            response = construct_head_response(server, client, request);
        } else {
            min = min(request->method->length, POST_LENGTH);
            if (strncmp(request->method->array, POST, (size_t) min) == 0) {
                response = construct_post_response(server, client, request);
            }
        }
    }
    // The client is requesting a method that we dont implement
    if (response == NULL) {
        response = construct_error(server, client, request);
    }
    // Return the response to the client
    write_response(client, response);
    // Log the event to the log file
    log_event(server, client, request, response);

    // Free the resources
    free_request(request);
    free_response(response);
}

// Reads a crlf terminated line
AWL *read_until_crlf(HE *client, int *result) {
    AWL *awl;
    alloc_awl(awl, 1)
    awl->length = 0;

    char buff[1];
    memset(&buff, 0, sizeof(char) * 1);
    while (1) {
        ssize_t read_t = read(client->socket, buff, 1);
        // 0 means that we reached the end of the file
        if (read_t == 0) {
            // Notify the caller that he cant keep reading
            *result = 0;
            return awl;
        } else if (read_t == -1) { // -1 -error while reading the socket
            *result = 1;
            error_out("ssize_t read_t \\r")
        } else {
            if (buff[0] == CR) { // check if the character we read is CR
                read_t = read(client->socket, buff, 1); // read the next character
                if (read_t == 0) { // if 0 then we reached the end
                    *result = 0;
                    return awl;
                } else if (read_t == -1) { // -1 -error while reading the socket
                    *result = 1;
                    error_out("ssize_t read_t \\n")
                } else {
                    // if LF is found then we are at the end of the line
                    // stop reading
                    if (buff[0] == LF) {
                        break;
                    }
                }

                // Its not end of the line, append CR we read to the line
                int prev_length = awl->length;
                ++awl->length;
                increase_char_array(awl->array, prev_length, awl->length)
                awl->array[prev_length] = CR;
            }
            // Append the character we read to the line
            int prev_length = awl->length;
            ++awl->length;
            increase_char_array(awl->array, prev_length, awl->length)
            awl->array[prev_length] = buff[0];
        }
    }
    // Notify the caller that he can continue reading and return the result
    *result = 1;
    return awl;
}

// Parse the request line and construct the HTTP request
HREQ *construct_request(AWL *request_line) {
    // Alloce the memory for the request
    HREQ *hreq = malloc(sizeof(HREQ));
    hreq->header = NULL;
    hreq->parameter = NULL;
    hreq->body = NULL;

    // Structur of HTTP request line: Method URL version
    // Find the method of the request
    int method = 0;
    for (; method < request_line->length; ++method) {
        if (request_line->array[method] == SP) {
            break;
        }
    }
    // Allocate the memory of the method
    alloc_awl(hreq->method, method)
    // Copy the positions that belong to the method
    strncpy(hreq->method->array, request_line->array, method);
    ++method;

    // Find the URL of the request
    int url = method;
    for (; url < request_line->length; ++url) {
        if (request_line->array[url] == SP) {
            break;
        }
    }
    // Allocate the memory of the URL
    alloc_awl(hreq->url, url - method)
    // Copy the positions that belong to the URL
    strncpy(hreq->url->array, request_line->array + method, hreq->url->length);
    ++url;

    // Find the version of the request
    int version = request_line->length - url;
    // Allocate the memory for the version
    alloc_awl(hreq->version, version)
    // Copy postitions of the version
    strncpy(hreq->version->array, request_line->array + url, version);

    return hreq;
}

// Parse the query parameters
void get_parameters_if_present(HREQ *request) {
    int parameter = 0;
    int has_parameter = 0;
    // Find the question mark in the URL
    for (; parameter < request->url->length; ++parameter) {
        if (request->url->array[parameter] == QM) {
            ++parameter;
            has_parameter = 1; // Boolean variable
            break;
        }
    }
    int fragment = 0;
    // Find the hashtag in the URL
    for (; fragment < request->url->length; ++fragment) {
        if (request->url->array[fragment] == FR) {
            break;
        }
    }

    // Check if we found a ? and that the fragment position is > the parameters position
    if (has_parameter && fragment > parameter) {
        // Allocate memory for the query
        AWL *query;
        alloc_awl(query, fragment - parameter)
        int start = parameter;
        // Copy the query
        for (; start < fragment; ++start) {
            query->array[start - parameter] = request->url->array[start];
        }

        int i, j;
        int equal;
        int block;

        for (i = 0; i < query->length; ++i) {
            equal = 0;
            block = 0;
            // Search for the name of paramerter, we know that the symbol = defines the end of name of the parameter
            while (i < query->length && query->array[i] != EQ) {
                ++equal;
                ++i;
            }
            ++i;
            // Search for the value of parameter, we know that the symbol &
            // or the end of the length defines the value of the parameter
            while (i < query->length && query->array[i] != AP) {
                ++block;
                ++i;
            }
            // Check if name and value have a valid length
            if (equal == 0 || block == 0) {
                break;
            }

            HP *hp = malloc(sizeof(HP));
            hp->next = NULL;
            // Allocate memory
            alloc_awl(hp->name, equal)
            alloc_awl(hp->value, block)

            int size = equal + block + 1;
            for (j = 0; j < equal; ++j) {
                // Copy the name of the parameter
                hp->name->array[j] = query->array[i - size + j];
            }
            size = block;
            for (j = 0; j < block; ++j) {
                // Copy the value of the parameter
                hp->value->array[j] = query->array[i - size + j];
            }

            // Append the parameter to the linked list of the request
            if (request->parameter == NULL) {
                request->parameter = hp;
            } else {
                HP *next = request->parameter;
                while (next->next != NULL) {
                    next = next->next;
                }
                next->next = hp;
            }
        }
    }
}

// Adds a header to the response
void add_header_to_response(HRES *response, AWL *request_line) {
    // Allocate memory for the header
    HH *header = malloc(sizeof(HH));
    header->next = NULL;

    int name = 0;
    // Searching for the space in the AWL that defines the end of the header name
    for (; name < request_line->length; ++name) {
        if (request_line->array[name] == SP) {
            break;
        }
    }
    --name;
    // Allocate memory for the header name
    alloc_awl(header->name, name)
    // Copy the header name
    strncpy(header->name->array, request_line->array, name);
    name += 2;

    // Allocate memory for the value of the header
    int value = request_line->length - name;
    alloc_awl(header->value, value)
    // Copy the value of the header
    strncpy(header->value->array, request_line->array + name, value);

    // Append the header to the response's linked list
    if (response->header == NULL) {
        response->header = header;
    } else {
        HH *next = response->header;
        while (next->next != NULL) {
            next = next->next;
        }
        next->next = header;
    }
}
// Find a parameter in the linked list
HP *find_parameter(HP *first, AWL *name) {
    int min;
    while (first != NULL) {
        min = min(first->name->length, name->length);
        if (strncasecmp(first->name->array, name->array, (size_t) min) == 0) {
            return first;
        }
        first = first->next;
    }
    return NULL;
}

// Add the header to the request's linked list
void add_header_to_request(HREQ *request, AWL *request_line) {
    // Allocate memory for the header
    HH *header = malloc(sizeof(HH));
    header->next = NULL;

    int name = 0;
    // Searching for the space in the AWL that defines the end of the header name
    for (; name < request_line->length; ++name) {
        if (request_line->array[name] == SP) {
            break;
        }
    }
    --name;
    // Allocate memory for the header name
    alloc_awl(header->name, name)
    // Copy the header name
    strncpy(header->name->array, request_line->array, name);
    name += 2;

    int value = request_line->length - name;
    // Allocate memory for the value of the header
    alloc_awl(header->value, value)
    // Copy the value of the header
    strncpy(header->value->array, request_line->array + name, value);

    // Append the header to the request's linked list
    if (request->header == NULL) {
        request->header = header;
    } else {
        HH *next = request->header;
        while (next->next != NULL) {
            next = next->next;
        }
        next->next = header;
    }
}

// Finds a header in a linked list
HH *find_header(HH *first, AWL *name) {
    int min;
    while (first != NULL) {
        min = min(first->name->length, name->length);
        if (strncasecmp(first->name->array, name->array, (size_t) min) == 0) {
            return first;
        }
        first = first->next;
    }
    return NULL;
}

// Retrieve the body of the request
void get_body_if_present(HE *client, HREQ *request) {
    AWL *cl;
    alloc_awl(cl, CL_LENGTH)
    strncpy(cl->array, CL, CL_LENGTH);
    // Search for the content length header
    HH *cl_header = find_header(request->header, cl);
    // If the hader is present then we can read the body
    if (cl_header != NULL) {
        int i = 0;
        int m = 1;
        int cl_length = 0;
        // Convert the header value to an integer
        for (; i < cl_header->value->length; ++i) {
            cl_length *= m;
            cl_length += (cl_header->value->array[i] - '0');
            m *= 10;
        }

        // Allocate memory for the body
        alloc_awl(request->body, cl_length)
        // Read the body
        read(client->socket, request->body->array, (size_t) cl_length);
    }
    // Deallocating the variable used to find the content length header
    dealloc_awl(cl)
}

// Write the reponse to the clients socket
void write_response(HE *client, HRES *response) {
    ssize_t bytes_written = write(client->socket, response->version->array, (size_t) response->version->length);
    if (bytes_written == -1) {
        error_out("Failed to write version")
    }

    bytes_written = write(client->socket, " ", 1);
    if (bytes_written == -1) {
        error_out("Failed to write version sp")
    }

    bytes_written = write(client->socket, response->status_code->array, (size_t) response->status_code->length);
    if (bytes_written == -1) {
        error_out("Failed to write status code")
    }

    bytes_written = write(client->socket, " ", 1);
    if (bytes_written == -1) {
        error_out("Failed to write status code sp")
    }

    bytes_written = write(client->socket, response->version->array, (size_t) response->version->length);
    if (bytes_written == -1) {
        error_out("Failed to write status message")
    }

    bytes_written = write(client->socket, " ", 1);
    if (bytes_written == -1) {
        error_out("Failed to write status message sp")
    }

    bytes_written = write(client->socket, "\r", 1);
    if (bytes_written == -1) {
        error_out("Failed to write CR")
    }

    bytes_written = write(client->socket, "\n", 1);
    if (bytes_written == -1) {
        error_out("Failed to write LF")
    }

    HH *header = response->header;
    while (header != NULL) {
        bytes_written = write(client->socket, header->name->array, (size_t) header->name->length);
        if (bytes_written == -1) {
            error_out("Failed to write header name")
        }

        bytes_written = write(client->socket, ": ", 2);
        if (bytes_written == -1) {
            error_out("Failed to write header colon sp")
        }

        bytes_written = write(client->socket, header->value->array, (size_t) header->value->length);
        if (bytes_written == -1) {
            error_out("Failed to write header value");
        }

        bytes_written = write(client->socket, "\r", 1);
        if (bytes_written == -1) {
            error_out("Failed to write CR");
        }

        bytes_written = write(client->socket, "\n", 1);
        if (bytes_written == -1) {
            error_out("Failed to write LF");
        }

        header = header->next;
    }

    bytes_written = write(client->socket, "\r", 1);
    if (bytes_written == -1) {
        error_out("Failed to write CR")
    }

    bytes_written = write(client->socket, "\n", 1);
    if (bytes_written == -1) {
        error_out("Failed to write LF")
    }

    if (response->body != NULL) {
        bytes_written = write(client->socket, response->body->array, (size_t) response->body->length);
        if (bytes_written == -1) {
            error_out("Failed to write body")
        }
    }
}

// Release the resources of the request
void free_request(HREQ *request) {
    dealloc_awl(request->method)
    dealloc_awl(request->url)
    dealloc_awl(request->version)
    free_parameter(request->parameter);
    free_header(request->header);
    if (request->body != NULL) {
        dealloc_awl(request->body)
    }
    free(request);
}

// Release the resources of the response
void free_response(HRES *response) {
    dealloc_awl(response->version)
    dealloc_awl(response->status_code)
    dealloc_awl(response->status_message)
    free_header(response->header);
    if (response->body != NULL) {
        dealloc_awl(response->body)
    }
    free(response);
}
// Release the resources of the parameters
void free_parameter(HP *parameter) {
    if (parameter != NULL) {
        dealloc_awl(parameter->name)
        dealloc_awl(parameter->value)
        free_parameter(parameter->next);
        free(parameter);
    }
}

// Release the resources of the headers
void free_header(HH *header) {
    if (header != NULL) {
        dealloc_awl(header->name)
        dealloc_awl(header->value)
        free_header(header->next);
        free(header);
    }
}

// Construct an error response
HRES *construct_error(HE *server, HE *client, HREQ *request) {
    HRES *res = malloc(sizeof(HRES));
    alloc_awl(res->version, request->version->length)
    strncpy(res->version->array, request->version->array, res->version->length);
    alloc_awl(res->status_code, MNA_CODE_LENGTH)
    strncpy(res->status_code->array, MNA_CODE, MNA_CODE_LENGTH);
    alloc_awl(res->status_message, MNA_MESSAGE_LENGTH)
    strncpy(res->status_message->array, MNA_MESSAGE, MNA_MESSAGE_LENGTH);

    char html_start[] = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"
            "<title>PA3_SERVER</title>"
            "</head>"
            "<body>"
            "<p>";
    size_t html_start_length = strlen(html_start);
    alloc_awl(res->body, html_start_length)
    strncpy(res->body->array, html_start, html_start_length);

    char html_content[] = "Method not supported";
    size_t html_content_length = strlen(html_content);
    append_to_awl(res->body, html_content, html_content_length)

    char html_end[] = "</p>"
            "</body>"
            "</html>";
    size_t html_end_length = strlen(html_end);
    append_to_awl(res->body, html_end, html_end_length)

    return res;
}

// Log the event to the log file
void log_event(HE *server, HE *client, HREQ *request, HRES *response) {
    FILE *log = fopen(LOG_FILENAME, LOG_FILENAME_OPEN_MODE);
    if (log != NULL) {
        char buff[24];
        memset(&buff, 0, sizeof(char) * 24);
        time_t now;
        time(&now);
        strftime(buff, sizeof buff, "%FT%TZ", gmtime(&now));

        char char_client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client->sock_addr.sin_addr), char_client_ip, INET_ADDRSTRLEN);

        char char_client_port[6];
        memset(&char_client_port, 0, sizeof(char) * 6);
        sprintf(char_client_port, "%i", ntohs(client->sock_addr.sin_port));

        fprintf(log, "%s : %s:%s %s %s : %s\n", buff, char_client_ip, char_client_port, request->method->array,
                request->url->array, response->status_code->array);
        fclose(log);
    }
}

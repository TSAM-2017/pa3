// Eliminating duplicates:
#ifndef C_HTTP_SERVER_HTTP_SPEC_H
#define C_HTTP_SERVER_HTTP_SPEC_H

#include <arpa/inet.h>
#include <netinet/in.h>
//#include <openssl/err.h>
//#include <openssl/ssl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

// Constant, structures and macros

// Special characters that http uses to identify operations/actions
#define CR '\r'
#define LF '\n' //CRLF end of line
#define SP ' '
#define QM '?' // Start of a query parameters
#define FR '#' // Start of the URI fragment
#define EQ '=' // For query parameters, left part is the variable name and the right part the value
#define AP '&' // Separate query parameters
#define SC ';' // For the cookies, similar as &

// Constants for the request methods
#define GET "GET"
#define GET_LENGTH 3

#define HEAD "HEAD"
#define HEAD_LENGTH 4

#define POST "POST"
#define POST_LENGTH 4

// Constants for the response codes and messages
#define OK_CODE "200"
#define OK_CODE_LENGTH 3
#define OK_MESSAGE "OK"
#define OK_MESSAGE_LENGTH 2

#define MNA_CODE "405"
#define MNA_CODE_LENGTH 3
#define MNA_MESSAGE "Method Not Allowed"
#define MNA_MESSAGE_LENGTH 18

// Contants for headers
#define CL "Content-Length"
#define CL_LENGTH 14

#define CT "Content-Type"
#define CT_LENGTH 12

#define TEXT_HTML "text/html"
#define TEXT_HTML_LENGTH 9

// For the log file name
#define LOG_FILENAME "log.txt"
#define LOG_FILENAME_OPEN_MODE "a"

// Boolean to enable returning the query
// parameters for all URIs or just the test URI
#define ENABLE_PARAMETERS_FOR_ALL 0

// Represents a server or a client
struct http_entity {
    int socket;
    struct sockaddr_in sock_addr;
};
// HE is a synonym for struct http_entity
typedef struct http_entity HE;

// Structure that has an array and the length of it
struct array_with_length {
    char *array;
    int length;
};
// AWL is a synonym for struct array_with_length
typedef struct array_with_length AWL;

// HTTP header is a name and a value
// This is a linked list
struct http_header {
    AWL *name;
    AWL *value;

    // Pointer to the next one in the list
    struct http_header *next;
};
// HH is a synonym for struct http_header
typedef struct http_header HH;

// The parameter of a query
// This is a linked list
struct http_parameter {
    AWL *name;
    AWL *value;

    // Pointer to the next one in the list
    struct http_parameter *next;
};
// HP is a synonym for struct htt_parameter
typedef struct http_parameter HP;

// The structure of the HTTP requests
struct http_request {
    // The HTTP method of the request
    AWL *method;
    // The URL of the request
    AWL *url;
    // The HTTP version of the request
    AWL *version;

    // The linked list of headers
    HH *header;
    // The linked list of query parameters
    HP *parameter;
    // The body of the request
    AWL *body;
};
// HREQ is a synonym for struct http_request
typedef struct http_request HREQ;

// The structure of the HTTP response
struct http_response {
    // The HTTP version of the response
    AWL *version;
    // The status code of the HTTP response
    AWL *status_code;
    // The status message of the HTTP response
    AWL *status_message;
    // The linked list of headers
    HH *header;
    // The body of the response
    AWL *body;
};
// HRES is a synonym for struct http_response
typedef struct http_response HRES;

// Macro to print an error and exit the program
// Takes an argument and prints it out
#define error_out(_args_) \
    { \
        perror(_args_); \
        exit(1); \
    }

// Macro to allocate a AWL structure with a size
// 1. allocating the AWL structure
// 2. clearing the memory for the structure
// 3. allocating the memory for the char array with the specified length
// 4. clearing the memory of the array
// 5. assigning the length to the structure
#define alloc_awl(_awl_, _awl_size_) \
    { \
        _awl_ = malloc(sizeof(AWL)); \
        memset(_awl_, 0, sizeof(AWL)); \
        _awl_->array = malloc(sizeof(char) * _awl_size_); \
        memset(_awl_->array, 0, sizeof(char) * _awl_size_); \
        _awl_->length = _awl_size_; \
    }

// Destroys the AWL structure
// 1. free the memory of the array
// 2. free the memory of the structure
#define dealloc_awl(_awl_) \
    { \
        free(_awl_->array); \
        free(_awl_); \
    }

// Increase the capacity of a char array
// 1. create a new array with the new length
// 2. clear the memory of the new array
// 3. copy the old array to the new array
// 4. free the memory of the old array
// 5. assign the old array to the new array
#define increase_char_array(_ica_array_, _ica_old_length_, _ica_length_) \
    { \
        char *_ica_new_array_ = malloc(sizeof(char) * _ica_length_); \
        memset(_ica_new_array_, 0, sizeof(char) * _ica_length_); \
        strncpy(_ica_new_array_, _ica_array_, _ica_old_length_); \
        free(_ica_array_); \
        _ica_array_ = _ica_new_array_; \
    }

// Append a char array to an AWL structure
// 1. call the increase char array macro to make space for the array to copy
// 2. increase the AWL structures length by the copied array length
// 3. append the array
#define append_to_awl(_ata_awl_, _ata_array_, _ata_length_) \
    { \
        increase_char_array(_ata_awl_->array, _ata_awl_->length, _ata_awl_->length + _ata_length_) \
        _ata_awl_->length += _ata_length_; \
        strncat(_ata_awl_->array, _ata_array_, _ata_length_); \
    }

// Get the smaller length of two values
#define min(_a_, _b_) \
   ({ typeof (_a_) _a = (_a_); \
       typeof (_b_) _b = (_b_); \
     _a < _b ? _a : _b; })

#endif //C_HTTP_SERVER_HTTP_SPEC_H

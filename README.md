# Programming Assignment 3 – HTTP server

Team:
Dagur Arinbjorn Danielsson <dagur13@ru.is>
Raquelita Ros Aguilar <raquelita15@ru.is>
Thordis Bakkmann Kristinsdottir <thordis15@ru.is>

how to build:
run 'make' within the 'src/' directory
or run 'make -C ./src' within the pa3/ directory

how to start server:
```
$ ./httpd <port>
```
all other signatures will cause errors

-----
### Requirements

**1. (2 points) The hand-in must conform to the following to be graded:**
- All necessary files need to be stored in an archive
- Provide stored passwords in a file
- You must have the generated keys in the top-level directory.
- Include ./AUTHORS file 
- Include ./README file 
- Include Makefile that compiles the httpd program.

**2. (2 points) It must be possible to run the server using the commands:**

[student15@skel pa2]$ make -C ./src

[student15@skel pa2]$ ./src/httpd $port $((port + 1))
   
**3. (2 points) Commented code in a meaningful way and use proper indentation.**

**4. (2 points) No crash, no memory leak and no unknown security issues.**

**5. Parse the arguments of the request and call functions to generate different content for each request.**
- 5.1. Parse the query component of the URI to supply arguments to the function.
- 5.2. Implement one test URI that, in addition to the fields displayed above, also displays the queries, one argument per line.
- 5.3. Implement second page, called color shall display an empty page, but use the value of the query bg as background colour.
(ex: http://localhost/color?bg=red returns a page with <body style="background-color:red"></body> as its body.)

**6. Cookies**

Parse the header of the requests and make the data relevant to users accessible in a useful manner.

**7. Key management**

To establish any connection via OpenSSL, you need to generate a certificate that is used to establish the connection.

**8. OpenSSL**

TODO

**9. Authentication**

TODO

-----
### Code structure
It was decided to break the project down in different C files for the simplicity of the development. It was uncertain in the beginning of the implementation on how big the code would end up, and therefor it was decided to break it down into different files for each request, specs and setup. 

A header file http_spec.h was created that contains C function declarations and macro definitions that is shared between several source files. Why macros? Well the short answer is better performance, it's perprocessed and less functions. It is of course possible to replace the macros in this project with functions, but functions can sometimes be more expensive as they have to locate space for a parameter, resolve addresses and so on. 

#### File Structure
- main.c - The input argument and calling the relevant functions
- http_spec.h - Contains constant, structures and macros
- http_server.c - The server setup
- http_get.c - The GET method
- http_post.c - The POST method
- http_head.c - The HEAD method



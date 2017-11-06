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
• All necessary files need to be stored in an archive
• Provide stored passwords in a file
• You must have the generated keys in the top-level directory.
• Include ./AUTHORS file 
• Include ./README file 
• Include Makefile that compiles the httpd program.

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

**8. OpenSSL**
TODO
**9. Authentication**
TODO
-----
### Code structure:



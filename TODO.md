## CGI
1. Remove hard coded binary path - DONE
2. Remove hard coded cgi script name - DONE
3. Remove hard coded cgi arguments - DONE
4. Improve error handling, use ServerLogger instead of perror - DONE
5. Handle case when infinite script is executed - DONE
6. Handle case when error inside of a CGI process occured - TODO
7. Add chunk support for CGI(Important)

Code Refactor:
## Event Handler Routine
### The Problem 
	Multiple routines for different FDs which creates branches in code
	that hard to maintain, read and scale.

### Solutions
	1. Move routines in classes and create Interface class for routines, then call the interface from server.
	Pros:
		1. Insanely easy to implement
		2. Much more readable code in Server::handle_request_event
		3. Easy to scale
		4. Updating the implementation does not affect any code other than its own, because it being called through an interface.xw
	Cons:
		1. Calling the interface may be ambiguous; you don't know what exactly will be executed.


### To fix
1. 2025-10-30 14:23:04 [RequestHeaderParser::parse_absolute_path] Error: failed to decode UTF-8 encoded bytes: '%20png'
2025-10-30 14:23:04 [RequestHeaderParser::parse_request_line] Error: failed to parse absolute path: '/Uploads/45060919_p0%20png'
2025-10-30 14:23:04 [RequestHeaderParser::parse_complete_header] Error: failed to parse request line: 'Host: 127.0.0.1
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:144.0) Gecko/20100101 Firefox/144.0
Accept: */*
Accept-Language: en-GB,en;q=0.5
Accept-Encoding: gzip, deflate, br, zstd
Referer: http://127.0.0.1/files.html
Origin: http://127.0.0.1
Connection: keep-alive
Sec-Fetch-Dest: empty
Sec-Fetch-Mode: cors
Sec-Fetch-Site: same-origin
Priority: u=0'

2. cgi

## Errors - Gosha
1. fix s3gfault when someone trying to upload a file
2. if you run webserv with ports that are lower than: 1000
3. if nginx is running in background then programm sudenly quits

## To check - Gosha
1. when i will fix s3gfault, try to upload multiple files from different ips at the same time.
2. check the case when Ahan's webserver is exited by ctrl + c and then our webserv is starting s3gfaulting

## To implement - Aboba(Everyone)
1. Logger - Gosha
2. CGI - Gosha(send to everyone info about CGI that Gosha will read)
3. POST, DELETE methods - Marianna
4. chuncked POST - Gosha
5. do we need more http response codes? - Marianna
6. tests(aboba) - EVERYONE

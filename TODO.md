## Bugs - Gosha
1. Fix s3gfault error when trying to upload a file - TODO: Andor
2. If the web server is running with ports below: 1000 - TODO: Gosha IDK???
3. If nginx is running in the background, the program suddenly quits - FIXED

## To check - Gosha
1. When I fix the s3gfault error, try uploading multiple files from different IP addresses at the same time.
2. Test the case when Ahan's web server closes with Ctrl+C, and then our web server starts s3gfaulting - TODO: Gosha

## Implement - Aboba (for everyone)
1. Logger - Gosha - Done
2. CGI - Gosha (send everyone information about CGI, which Gosha can read)
3. POST, DELETE methods - Marianna
4. Chunked POST - Marianna(sadly, but this part is about Request)
5. Do we need more HTTP response codes? - Marianna
6. tests(aboba) - for everyone
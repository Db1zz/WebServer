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

/*
	each read() call can return different amount of readed bytes, and this issue can cause
	in incomplete headers, data content and boundary lables, to prevent it, we can store important
	content such as headers and boundary lables in a body_chunk till the content will be complete.

	start
	------geckoformboundarydbe55acddb6231da461629742a34e1e6\r\n
	Content-Disposition: form-data; name="file"; filename="upload.html"\r\n
	Content-Type: text/html\r\n
	\r\n
	data
	bla bla bla\r\n
	------geckoformboundarydbe55acddb6231da461629742a34e1e6--\r\n
	cache:
	fadfafdfddsadadsfadafsdfssfdaasdfdfsadfsdsadsadfsdblafadd\r\n------geckoformboundarydbe55acddb6231da461629742a34e1e6--\r\n asdfasdfasfasdf

	just readed:

	cache.size() + just_readed.size() = 24
	boundary.size() = 26
	if (cache.size() + just_readed.size() < boundary.size())
		append to cache
	else {
		cache.erase(0, cache.size() + just_readed.size() - boundary.size());
	}

	The end of the start boundary can be found by \r\n\r\n
	and
	the end boundary by --\r\n on the end
	use tail_bytes as safety thing to store boundary parts.
*/
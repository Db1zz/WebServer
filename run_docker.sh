#!/bin/bash
docker build -t webserv-dev .
docker run --rm -it -p 80:80 -v "$PWD":/app -w /app webserv-dev bash
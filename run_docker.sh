#!/bin/bash
docker build -t webserv-dev .
docker run --rm -it -p 8080:8080 -v "$PWD":/app -w /app webserv-dev bash
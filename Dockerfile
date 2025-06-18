FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
	build-essential \
	g++ \
	make \
	&& rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN make re

CMD ["./webserv"]
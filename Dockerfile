FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
	build-essential \
	g++ \
	make

WORKDIR /app

CMD ["bash"]
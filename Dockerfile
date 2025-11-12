FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
	build-essential \
	g++ \
	make \
	nginx \
	valgrind \
	python3 \
	pip \
	python3.12-venv \
	cmake \
	man-db

WORKDIR /app

CMD ["bash"]
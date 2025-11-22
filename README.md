# aboba.<b>webserv</b>

##  Introduction
**aboba.webserv** is a lightweight, single-threaded web server implemented using outdated **C++98**(cringe) and low-level **Linux kernel APIs**.  
Despite its intentionally old-school tech stack, the server is efficient, standards-compliant, and fun to hack on xd

##  Features
- Fully supports **HTTP/1.1**
- **Nginx-style configuration** system
- **CGI** execution support
- **UTF-8** encoding/decoding
- Docker workflow for macOS users

##  Requirements
- `make`
- Linux system **or** Docker (for macOS users. Windows? idk who's uses **that**)

##  Installation & Usage

### 1. Clone the repository
- `git clone https://github.com/Db1zz/aboba.webserv`

### 2. Server goes brrrrrrrrrr
#### For Linux enjoyers
- `make`

#### For macOS
- `make run` note: your docker should be up, otherwise you'll get an error
- `make` in a docker container

## Done! Please be carefull with it <3

## Contributors:
https://github.com/Db1zz - Gosha
https://github.com/fraumarzhuk - Marianna
https://github.com/tamasandor - Andor


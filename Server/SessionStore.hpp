#ifndef SERVER_SESSION_STORE_HPP
#define SERVER_SESSION_STORE_HPP

#include <string>
#include <map>
#include <ctime>

class SessionStore {
public:
	struct Session {
		std::string username;
		std::time_t expires_at;
	};

	SessionStore();
	std::string create_session(const std::string& username, int ttl_seconds);
	bool get_username(const std::string& session_id, std::string& out_username);
	void destroy_session(const std::string& session_id);

private:
	std::map<std::string, Session> _store;
};

#endif // SERVER_SESSION_STORE_HPP

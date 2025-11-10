#include "SessionStore.hpp"
#include "../Utilities/hash.hpp"

#include <sstream>

SessionStore::SessionStore() {}

std::string SessionStore::create_session(const std::string& username, int ttl_seconds) {
	std::string sid = HashGenerator::generate(32);
	std::time_t now = std::time(NULL);
	Session s;
	s.username = username;
	s.expires_at = now + ttl_seconds;
	_store[sid] = s;
	return sid;
}

bool SessionStore::get_username(const std::string& session_id, std::string& out_username) {
	std::map<std::string, Session>::iterator it = _store.find(session_id);
	if (it == _store.end()) {
		return false;
	}
	std::time_t now = std::time(NULL);
	if (it->second.expires_at < now) {
		_store.erase(it);
		return false;
	}
	out_username = it->second.username;
	return true;
}

void SessionStore::destroy_session(const std::string& session_id) {
	std::map<std::string, Session>::iterator it = _store.find(session_id);
	if (it != _store.end()) {
		_store.erase(it);
	}
}

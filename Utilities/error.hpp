#ifndef UTILITIES_ERROR_HPP_
#define UTILITIES_ERROR_HPP_

#include <string>

class Error {
   public:
	enum Code { eNoError, eError, eDataIsNotReady, eFatalError };

	Error();
	Error(Code code);
	Error(Code code, const std::string& msg);
	~Error() {}

	operator bool() const;
	bool is_ok() const;
	bool is_fatal() const;
	Error code() const;
	bool operator==(Code code) const;
	bool operator==(const Error& code) const;

	const std::string& msg() const;
	void set_msg(const std::string& msg);

   private:
	Code _error_code;
	std::string _msg;
};

#endif // UTILITIES_ERROR_HPP_
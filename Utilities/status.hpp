#ifndef WEBSERVER_UTILITIES_STATUS_HPP
#define WEBSERVER_UTILITIES_STATUS_HPP

#include <sstream>
#include <string>

enum ErrorCode {
	// HTTP Error Codes
	// 1xx
	Continue = 100,
	SwitchingProtocols = 101,
	Processing = 102,

	// 2xx
	OK = 200,
	Created = 201,
	Accepted = 202,
	NoContent = 204,
	ResetContent = 205,
	PartitialContent = 206,

	// 3xx
	MultipleChoices = 300,
	MovedPermanently = 301,
	Found = 302,
	SeeOther = 303,
	NotModified = 304,
	UseProxy = 305,
	Unused = 306,
	TemporaryRedirect = 307,

	// 4xx
	BadRequest = 400,
	Unauthorized = 401,
	Forbidden = 403,
	NotFound = 404,
	MethodNotAllowed = 405,
	NotAcceptable = 406,
	ProxyAuthenticationRequired = 407,
	RequestTimeout = 408,
	Conflict = 409,
	Gone = 410,
	LengthRequired = 411,
	PreconditionFailed = 412,
	RequestEntityTooLarge = 413,
	RequestURITooLong = 414,
	UnsupportedMediaType = 415,
	RequestedRangeNotSatisfiable = 416,
	ExpectationFailed = 417,
	RequestHeaderFieldsTooLarge = 431,

	// 5xx
	InternalServerError = 500,
	NotImplemented = 501,
	BadGateway = 502,
	ServiceUnavailable = 503,
	GatewayTimeout = 504,
	HTTPVersionNotSupported = 505,
	// END HTTP Error Codes

	// 6xx to 9xx - Internal Server Errors / Control codes
	Incomplete = 600, // need more data
	IncompleteRequestHeader = 601,
	IncompleteRequestBodyHeader = 602,
	NoMime = 603,
	NoFilename = 604,
	RequestBoundaryIsNotProvided = 605,
	EndBoundaryNotFound = 606,
	InvalidPath = 607,
	CloseConnection = 608,
	InvalidFilenameFormat = 609,
	DataIsNotReady = 610,
	DataIsReady = 611,

	// 1xxx - Special error codes
	// !!! DO NOT ADD ANYTHING BELOW THIS LINE !!!
	UnknownError = 998,
	Interrupted = 999,
	CustomError = 1000
};

class Status {
   public:
	Status();
	Status(const std::string& error_msg); 
	Status(ErrorCode error_type, int error_code, const std::string& error_msg,
		   bool is_error = false);

	Status(const Status& to_copy);
	Status& operator=(const Status& to_copy);

#define DEFINE_STATUS_FACTORY(NAME) \
	static Status NAME() { return Status(::NAME, (int) ::NAME, std::string(#NAME)); }

	// 1xx
	DEFINE_STATUS_FACTORY(Continue)
	DEFINE_STATUS_FACTORY(SwitchingProtocols)
	DEFINE_STATUS_FACTORY(Processing)

	// 2xx
	DEFINE_STATUS_FACTORY(OK)
	DEFINE_STATUS_FACTORY(Created)
	DEFINE_STATUS_FACTORY(Accepted)
	DEFINE_STATUS_FACTORY(NoContent)
	DEFINE_STATUS_FACTORY(ResetContent)
	DEFINE_STATUS_FACTORY(PartitialContent)

	// 3xx
	DEFINE_STATUS_FACTORY(MultipleChoices)
	DEFINE_STATUS_FACTORY(MovedPermanently)
	DEFINE_STATUS_FACTORY(Found)
	DEFINE_STATUS_FACTORY(SeeOther)
	DEFINE_STATUS_FACTORY(NotModified)
	DEFINE_STATUS_FACTORY(UseProxy)
	DEFINE_STATUS_FACTORY(Unused)
	DEFINE_STATUS_FACTORY(TemporaryRedirect)

	// 4xx
	DEFINE_STATUS_FACTORY(BadRequest)
	DEFINE_STATUS_FACTORY(Unauthorized)
	DEFINE_STATUS_FACTORY(Forbidden)
	DEFINE_STATUS_FACTORY(NotFound)
	DEFINE_STATUS_FACTORY(MethodNotAllowed)
	DEFINE_STATUS_FACTORY(NotAcceptable)
	DEFINE_STATUS_FACTORY(ProxyAuthenticationRequired)
	DEFINE_STATUS_FACTORY(RequestTimeout)
	DEFINE_STATUS_FACTORY(Conflict)
	DEFINE_STATUS_FACTORY(Gone)
	DEFINE_STATUS_FACTORY(LengthRequired)
	DEFINE_STATUS_FACTORY(PreconditionFailed)
	DEFINE_STATUS_FACTORY(RequestEntityTooLarge)
	DEFINE_STATUS_FACTORY(RequestURITooLong)
	DEFINE_STATUS_FACTORY(UnsupportedMediaType)
	DEFINE_STATUS_FACTORY(RequestedRangeNotSatisfiable)
	DEFINE_STATUS_FACTORY(ExpectationFailed)

	// 5xx
	DEFINE_STATUS_FACTORY(InternalServerError)
	DEFINE_STATUS_FACTORY(NotImplemented)
	DEFINE_STATUS_FACTORY(BadGateway)
	DEFINE_STATUS_FACTORY(ServiceUnavailable)
	DEFINE_STATUS_FACTORY(GatewayTimeout)
	DEFINE_STATUS_FACTORY(HTTPVersionNotSupported)
	DEFINE_STATUS_FACTORY(RequestHeaderFieldsTooLarge)

#undef DEFINE_STATUS_FACTORY

	// Internal control / helper factories: allow custom message
#define DEFINE_STATUS_FACTORY_MSG(NAME, MSG) \
	static Status NAME() { return Status(::NAME, (int) ::NAME, std::string(MSG)); }

	DEFINE_STATUS_FACTORY_MSG(Incomplete, "not enough data")
	DEFINE_STATUS_FACTORY_MSG(IncompleteRequestBodyHeader, "request body header is incomplete")
	DEFINE_STATUS_FACTORY_MSG(RequestBoundaryIsNotProvided, "request boundary is not provided")
	DEFINE_STATUS_FACTORY_MSG(NoMime, "mime not found")
	DEFINE_STATUS_FACTORY_MSG(NoFilename, "filename not found")
	DEFINE_STATUS_FACTORY_MSG(EndBoundaryNotFound, "end boundary not found")
	DEFINE_STATUS_FACTORY_MSG(InvalidPath, "invalid path")

	DEFINE_STATUS_FACTORY_MSG(Interrupted, "EINTR signal sent")
	DEFINE_STATUS_FACTORY_MSG(CloseConnection, "closed connection")
	DEFINE_STATUS_FACTORY_MSG(InvalidFilenameFormat, "invalid filename format");
	DEFINE_STATUS_FACTORY_MSG(DataIsNotReady, "the data is not ready for processing");
	DEFINE_STATUS_FACTORY_MSG(DataIsReady, "the data is ready for processing");
#undef DEFINE_STATUS_FACTORY_MSG

	// comparisons
	bool operator==(int code) const;
	bool operator==(ErrorCode error_type) const;
	bool operator==(const Status& status) const;

	// bool semantics: true == success, false == failure
	operator bool() const;
	bool is_ok() const;

	// getters
	const std::string& msg() const;
	ErrorCode error() const; // enum type
	int code() const;		 // numeric code — use this when comparing to errno constants

	// DEPRECATED
	void set_status_line(int error_code, std::string error_msg);
	const std::string status_line();

   private:
	ErrorCode _error_type;
	int _error_code; // numeric code
	std::string _error_msg;
	std::string _status_line;
	bool _is_error; // false == success, true == failure

	// helper
	static bool default_is_error_for_code(int code);
};

#endif // WEBSERVER_UTILITIES_STATUS_HPP

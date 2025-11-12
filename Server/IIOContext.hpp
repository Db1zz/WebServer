#ifndef SERVER_I_IO_CONTEXT_HPP_
#define SERVER_I_IO_CONTEXT_HPP_

class IIOContext {
public:
	virtual ~IIOContext() {};
	virtual void reset() = 0;
};

#endif  // SERVER_I_IO_CONTEXT_HPP_
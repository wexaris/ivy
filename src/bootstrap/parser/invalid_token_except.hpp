#pragma once
#include "util/span.hpp"
#include <exception>
#include <string>

/* An exception type for Invalid tokens. */
class InvalidToken : public std::exception
{
public:
	explicit InvalidToken(const std::string& message)
		: msg(message)
	{}

	virtual ~InvalidToken() throw () {}

	virtual const char* what() const noexcept {
		return msg.c_str();
	}

protected:
	std::string msg;
};
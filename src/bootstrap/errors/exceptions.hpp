#pragma once
#include <exception>

class CompilerException : public std::exception {

public:
	CompilerException() = default;
	virtual ~CompilerException() override = default;
};

class ErrorException : public CompilerException {

public:
	ErrorException() = default;
	virtual ~ErrorException() override = default;
};

class FatalException : public CompilerException {

public:
	FatalException() = default;
	virtual ~FatalException() override = default;
};

class InternalException : public CompilerException {

public:
	InternalException() = default;
	virtual ~InternalException() override = default;
};
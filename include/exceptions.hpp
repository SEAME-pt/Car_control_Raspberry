#pragma once

#include <exception>
#include <string>



class InitException : public std::exception {
			
public:
	InitException(const std::string& msg) : _message(msg) {}
			
	const char* what() const throw() {
		return _message.c_str();
	}
private:
	std::string	_message;
};

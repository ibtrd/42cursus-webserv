#include "ConfFile.hpp"
#include "ft.hpp"

std::string ConfFile::_unkwownDirective(const ConfToken &error) const {
	std::stringstream ss;
	ss << "unknown directive " << ft::quoted(error.str()) << " in " << this->_path << ":"
	   << error.line();
	return ss.str();
}

std::string ConfFile::_missingOpening(const ConfToken &error, const char c) const {
	std::stringstream ss;
	ss << "directive " << ft::quoted(error.str()) << " has no opening " << ft::quoted(c) << " in "
	   << this->_path << ":" << error.line();
	return ss.str();
}

std::string ConfFile::_invalidArgumentNumber(const ConfToken &error) const {
	std::stringstream ss;
	ss << "invalid number of arguments in " << ft::quoted(error.str()) << " directive in "
	   << this->_path << ":" << error.line();
	return ss.str();
}

std::string ConfFile::_unexpectedEOF(const ConfToken &error, const char c) const {
	std::stringstream ss;
	ss << "unexpected end of file, expecting " << ft::quoted(c) << " in " << this->_path << ":"
	   << error.line();
	return ss.str();
}

std::string ConfFile::_unexpectedEOF(const ConfToken &error, const char c1, const char c2) const {
	std::stringstream ss;
	ss << "unexpected end of file, expecting " << ft::quoted(c1) << " or " << ft::quoted(c2)
	   << " in " << this->_path << ":" << error.line();
	return ss.str();
}

std::string ConfFile::_unexpectedToken(const ConfToken &error) const {
	std::stringstream ss;
	ss << "unexpected " << ft::quoted(error.str()) << " in " << this->_path << ":" << error.line();
	return ss.str();
}

std::string ConfFile::_hostNotFound(const ConfToken &error, const ConfToken &host) const {
	std::stringstream ss;
	ss << "host not found in " << ft::quoted(host.str()) << " of the " << ft::quoted(error.str())
	   << " directive in " << this->_path << ":" << error.line();
	return ss.str();
}

std::string ConfFile::_invalidMethod(const ConfToken &method) const {
	std::stringstream ss;
	ss << "invalid method " << ft::quoted(method.str()) << " in " << this->_path << ":"
	   << method.line();
	return ss.str();
}

std::string ConfFile::_invalidValue(const ConfToken &directive, const ConfToken &inval) const {
	std::stringstream ss;
	ss << ft::quoted(directive.str()) << " directive invalid value in " << this->_path << ":"
	   << inval.line();
	return ss.str();
}

std::string ConfFile::_invalidValue(const ConfToken &directive, const ConfToken &inval,
                                    const std::string &expected1,
                                    const std::string &expected2) const {
	std::stringstream ss;
	ss << "invalid value " << ft::quoted(inval.str()) << " in " << ft::quoted(directive.str())
	   << " directive, it must be " << ft::quoted(expected1) << " or " << ft::quoted(expected2)
	   << " in " << this->_path << ":" << inval.line();
	return ss.str();
}

std::string ConfFile::_invalidPath(const ConfToken &directive, const ConfToken &path) const {
	std::stringstream ss;
	ss << "invalid path " << ft::quoted(path.str()) << " in " << ft::quoted(directive.str())
	   << " directive, it must start with " << ft::quoted('/') << " in " << this->_path << ":"
	   << path.line();
	return ss.str();
}

#include <unistd.h>
#include <string.h>

#include "Client.hpp"

error_t ARequest::_readCGI(void) {
	uint8_t buffer[REQ_BUFFER_SIZE];

	// static int compteur = 0;

	// std::cerr << "Request _readCGI: " << compteur++ << std::endl;

	ssize_t bytes = read(this->_context.cgiSockets[PARENT_SOCKET], buffer, REQ_BUFFER_SIZE);
	if (bytes == 0) {
		// std::cerr << "read: EOF" << std::endl;
		SET_REQ_WORK_COMPLETE(this->_context.requestState);
		return (REQ_CONTINUE);
	}
	if (bytes == -1) {
		std::cerr << "read: " << strerror(errno) << std::endl;
		// this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		SET_REQ_WORK_COMPLETE(this->_context.requestState);
		return (REQ_ERROR);
	}

	// std::cerr << "Request _readCGI: " << bytes << std::endl;
	// std::cerr << "Request _readCGI: |";
	// for (ssize_t i = 0; i < bytes; ++i) {
	// 	std::cerr << buffer[i];
	// }
	// std::cerr << "|" << std::endl;
	BinaryBuffer bBuffer(buffer, bytes);
	if (!IS_REQ_CGI_HEADERS_COMPLETE(this->_context.requestState)) {
		this->_parseCGIHeaders(bBuffer);
	}
	this->_context.responseBuffer.append(bBuffer);
	return (REQ_CONTINUE);
}

static status_code_t cgiStatusToStatus(std::string &statusHeader) {
	if (statusHeader.size() < 5 && statusHeader[3] == ' ' && std::isalpha(statusHeader[4])) {
		return (STATUS_INTERNAL_SERVER_ERROR);
	}
	if (!std::isdigit(statusHeader[0]) || !std::isdigit(statusHeader[1]) || !std::isdigit(statusHeader[2])) {
		return (STATUS_INTERNAL_SERVER_ERROR);
	}

	status_code_t code = (statusHeader[0] - '0') * 100 + (statusHeader[1] - '0') * 10 + (statusHeader[2] - '0');
	statusHeader.erase(0, 4);
	return (code);
}

void ARequest::_parseCGIHeaders(BinaryBuffer &buffer) {
	size_t      pos;
	std::string line;
	std::string key;
	std::string value;

	// std::cerr << "Parsing headers..." << std::endl;
	while ((pos = buffer.find("\r\n")) != std::string::npos) {
		line = buffer.substr(0, pos);
		buffer.erase(0, pos + 2);
		if (line.empty()) {
			headers_t::iterator statusHeader = this->_context.response.header(HEADER_STATUS);
			if (statusHeader == this->_context.response.headersEnd()) {
				this->_context.response.setStatusCode(STATUS_OK);
			} else {
				this->_context.response.setStatusCode(cgiStatusToStatus(statusHeader->second));
				this->_context.response.setReasonPhrase(statusHeader->second);
				this->_context.response.deleteHeader(HEADER_STATUS);
			}
			SET_REQ_CGI_HEADERS_COMPLETE(this->_context.requestState);
			this->_context.response.clearBody();
			this->_context.responseBuffer = this->_context.response.response();
			return;
		}
		pos = line.find(": ");
		if (pos == std::string::npos) {
			SET_REQ_CGI_HEADERS_COMPLETE(this->_context.requestState);
			SET_REQ_CGI_IN_COMPLETE(this->_context.requestState);
			this->_context.response.clear();
			this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
			this->_context.responseBuffer = this->_context.response.response();
			return;
		}
		key                         = line.substr(0, pos);
		value                       = line.substr(pos + 2);
		this->_context.response.setHeader(key, value);
		// std::cerr << "cgiHeader: |" << key << "| |" << value << "|" << std::endl;
	}
	return;
}

/* ************************************************************************** */

error_t ARequest::CGIIn(void) {
	throw std::logic_error("ARequest::CGIIn should not be called");
	return (REQ_ERROR);
}

error_t ARequest::CGIOut(void) {
	throw std::logic_error("ARequest::CGIOut should not be called");
	return (REQ_ERROR);
}


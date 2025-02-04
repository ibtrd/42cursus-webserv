#include <string.h>
#include <unistd.h>

#include "Client.hpp"

error_t ARequest::_readCGI(void) {
	uint8_t buffer[REQ_BUFFER_SIZE];

	// static int compteur = 0;

	// std::cerr << "Request _readCGI: " << compteur++ << std::endl;

	ssize_t bytes =
	    recv(this->_context.cgiSockets[PARENT_SOCKET], buffer, REQ_BUFFER_SIZE, MSG_NOSIGNAL);
	if (bytes == 0) {
		SET_REQ_WORK_COMPLETE(this->_context.requestState);
		if (!IS_REQ_CGI_HEADERS_COMPLETE(this->_context.requestState)) {
			this->_context.response.clear();
			this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
			this->_context.responseBuffer = this->_context.response.response();
		}
		return (REQ_CONTINUE);
	}
	if (bytes == -1) {
		std::cerr << "Error: read(): " << strerror(errno) << std::endl;
		// this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		SET_REQ_WORK_COMPLETE(this->_context.requestState);
		return (REQ_ERROR);
	}

	// std::cerr << "ARequest _readCGI: " << bytes << std::endl;
	// std::cerr << "ARequest _readCGI: |";
	// for (ssize_t i = 0; i < bytes; ++i) {
	// 	std::cerr << buffer[i];
	// }
	// std::cerr << "|" << std::endl;
	this->_readBuffer.append(buffer, bytes);
	// std::cerr << "Request _readCGI as bBuffer: |" << this->_readBuffer << "|" << std::endl;
	if (!IS_REQ_CGI_HEADERS_COMPLETE(this->_context.requestState)) {
		this->_parseCGIHeaders();
	}
	// std::cerr << "Request _readCGI after header: |" << this->_readBuffer << "|" << std::endl;
	if (IS_REQ_CGI_HEADERS_COMPLETE(this->_context.requestState)) {
		this->_context.responseBuffer.append(this->_readBuffer);
		this->_readBuffer.clear();
	}
	return (REQ_CONTINUE);
}

static status_code_t cgiStatusToStatus(std::string &statusHeader) {
	if (statusHeader.size() < 5 && statusHeader[3] == ' ' && std::isalpha(statusHeader[4])) {
		return (STATUS_INTERNAL_SERVER_ERROR);
	}
	if (!std::isdigit(statusHeader[0]) || !std::isdigit(statusHeader[1]) ||
	    !std::isdigit(statusHeader[2])) {
		return (STATUS_INTERNAL_SERVER_ERROR);
	}

	status_code_t code =
	    (statusHeader[0] - '0') * 100 + (statusHeader[1] - '0') * 10 + (statusHeader[2] - '0');
	statusHeader.erase(0, 4);
	return (code);
}

void ARequest::_parseCGIHeaders(void) {
	size_t      pos;
	std::string line;
	std::string key;
	std::string value;

	while ((pos = this->_readBuffer.find("\r\n")) != std::string::npos) {
		line = this->_readBuffer.substr(0, pos);
		this->_readBuffer.erase(0, pos + 2);
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
		key   = line.substr(0, pos);
		value = line.substr(pos + 2);
		this->_context.response.setHeader(key, value);
	}
	return;
}

/* ************************************************************************** */

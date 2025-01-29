#include "RequestPUT.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "ft.hpp"

/* CONSTRUCTORS ************************************************************* */

// RequestPUT::RequestPUT(void)
// {
// 	// std::cerr << "RequestPUT created" << std::endl;
// }

RequestPUT::RequestPUT(RequestContext_t &context)
    : ARequest(context), _chunked(false), _contentLength(0) {
	// std::cerr << "RequestPUT created" << std::endl;
	SET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);	// No workOut needed
	SET_REQ_CGI_IN_COMPLETE(this->_context.requestState);	// TMP
	SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);	// TMP
	this->_contentTotalLength = 0;
}

RequestPUT::RequestPUT(const RequestPUT &other)
    : ARequest(other), _chunked(false), _contentLength(0) {
	// std::cerr << "RequestPUT copy" << std::endl;
	*this = other;
}

RequestPUT::~RequestPUT(void) {
	// std::cerr << "RequestPUT destroyed" << std::endl;
	if (this->_file.is_open()) {
		this->_file.close();
	}
	if (0 == this->_tmpFilename.access(F_OK)) {
		std::remove(this->_tmpFilename.c_str());
	}
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestPUT &RequestPUT::operator=(const RequestPUT &other) {
	// std::cerr << "RequestPUT assign" << std::endl;
	(void)other;
	return (*this);
}

/* ************************************************************************** */

error_t RequestPUT::_generateFilename(void) {
	std::string basename = this->_context.ruleBlock->clientBodyTempPath().string() + this->_path.notdir();
	std::string tmp; 
	int32_t     i = 0;

	if (0 != this->_context.ruleBlock->clientBodyTempPath().access(W_OK)) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return (REQ_DONE);
	}

	do {
		tmp = basename + '.' + ft::generateRandomString(8) + ".tmp";
		++i;
	} while (0 == access(tmp.c_str(), F_OK) && i < 100);
	if (i == 100) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return (REQ_DONE);
	}
	this->_tmpFilename = tmp;
	return (REQ_CONTINUE);
}

void RequestPUT::_openFile(void) {
	if (REQ_CONTINUE != this->_generateFilename()) {
		return;
	}
	this->_file.open(this->_tmpFilename.c_str(),
	                 std::ios::out | std::ios::trunc | std::ios::binary);
	if (!this->_file.is_open()) {
		std::cerr << "open(): " << std::strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
	}
}

error_t RequestPUT::_checkHeaders(void) {
	headers_t::const_iterator it = this->_context.headers.find(HEADER_CONTENT_LENGTH);
	if (it == this->_context.headers.end()) {
		it = this->_context.headers.find(HEADER_TRANSFER_ENCODING);
		if (it == this->_context.headers.end()) {
			this->_context.response.setStatusCode(STATUS_LENGTH_REQUIRED);
			return (REQ_DONE);
		}
		if (it->second != HEADER_TRANSFER_CHUNKE) {
			this->_context.response.setStatusCode(STATUS_UNSUPPORTED_MEDIA_TYPE);
			return (REQ_DONE);
		}
		this->_chunked = true;
	}
	if (!this->_chunked) {
		this->_contentLength = sToContentLength(it->second, false);
		if (this->_contentLength == CONTENT_LENGTH_INVALID) {
			this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
			return (REQ_DONE);
		}
		if (this->_contentLength == CONTENT_LENGTH_TOO_LARGE ||
		    this->_contentLength > this->_context.ruleBlock->getMaxBodySize()) {
			this->_context.response.setStatusCode(STATUS_PAYLOAD_TOO_LARGE);
			return (REQ_DONE);
		}
		if (this->_contentLength == 0) {
			this->_context.response.setStatusCode(STATUS_NO_CONTENT);
			return (REQ_DONE);
		}
	} else {
		this->_contentLength = -1;
	}
	return (REQ_CONTINUE);
}

void RequestPUT::_saveFile(void) {
	// std::cerr << "RequestPUT _saveFile" << std::endl;
	this->_file.close();
	this->_context.response.setStatusCode(STATUS_CREATED);
	if (0 == this->_path.access(F_OK)) {
		if (0 != std::remove(this->_path.c_str())) {
			this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		}
	}
	if (0 != std::rename(this->_tmpFilename.c_str(), this->_path.c_str())) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
	}
	std::remove(this->_tmpFilename.c_str());
}

error_t RequestPUT::_readContent(void) {
	if (this->_contentLength - static_cast<int32_t>(this->_context.buffer.size()) >= 0) {
		this->_file.write(this->_context.buffer.c_str(), this->_context.buffer.size());
		this->_contentLength -= this->_context.buffer.size();
		this->_context.buffer.clear();
		// std::cerr << ".";	// DEBUG
	} else {
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	if (this->_contentLength == 0) {
		this->_saveFile();
		SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	return (REQ_CONTINUE);
}
/*
AAAAAAAAAABBBBBBBBBBCCCCCCCCCCDDDDDDDDDDEEEEEEEEEEFFFFFFFFFFGGGGGGGGGGHHHHHHHHHHIIIIIIIIIIJJJJJJJJJJKKKKKKKKKKLLLLLLLLLMMMMMMMMMNNNNNNNNNOOOOOOOOOPPPPPPPPP
*/
error_t RequestPUT::_readChunked(void) {
	// std::cerr << "RequestPUT _readChunked" << std::endl;
	while (!this->_context.buffer.empty()) {
		// std::cerr << "RequestPUT begin buffer: |" << this->_context.buffer << "|" << std::endl;

		// Read end of chunk
		if (this->_contentLength == 0) {
			if (this->_context.buffer.size() >= 2) {
				size_t pos = this->_context.buffer.rfind("\r\n", 0);
				if (pos == std::string::npos) {
					this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
					SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
					return (REQ_DONE);
				}
				this->_context.buffer.erase(0, pos + 2);
				this->_contentLength = -1;
			} else if (this->_context.buffer.size() == 1 && this->_context.buffer[0] != '\r') {
				this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
				SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
				return (REQ_DONE);
			} else {
				return (REQ_CONTINUE);
			}
		}

		// Read chunk size
		if (-1 == this->_contentLength) {
			// std::cerr << "RequestPUT contentLength -1" << std::endl;
			size_t pos = this->_context.buffer.find("\r\n");
			if (pos == std::string::npos) {
				return (REQ_CONTINUE);
			}
			std::string line = this->_context.buffer.substr(0, pos);
			if (line.empty()) {  // refuse empty chunk
				this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
				SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
				return (REQ_DONE);
			}
			// this->_context.buffer.erase(0, pos + 2);
			this->_contentLength = sToContentLength(line, true);
			if (this->_contentLength == CONTENT_LENGTH_INVALID) {
				this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
				SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
				return (REQ_DONE);
			}
			// Read end of transfer
			if (this->_contentLength == 0) {
				if (0 == this->_context.buffer.compare(ARequest::_chunkTerminator[0])) {
					this->_saveFile();
				} else {
					for (size_t i = 1; i < CHUNK_TERMINATOR_SIZE; ++i) {
						if (0 == this->_context.buffer.compare(ARequest::_chunkTerminator[i])) {
							this->_contentLength = -1;
							return (REQ_CONTINUE);
						}
					}
					this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
				}

				SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
				return (REQ_DONE);
			}
			this->_context.buffer.erase(0, pos + 2);
			this->_contentTotalLength += this->_contentLength;
			if (this->_contentLength == CONTENT_LENGTH_TOO_LARGE ||
			    this->_contentLength > this->_context.ruleBlock->getMaxBodySize() ||
			    this->_contentTotalLength > this->_context.ruleBlock->getMaxBodySize()) {
				this->_context.response.setStatusCode(STATUS_PAYLOAD_TOO_LARGE);
				SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
				return (REQ_DONE);
			}
		}
		// std::cerr << "RequestPUT contentLength: " << this->_contentLength << std::endl;
		// std::cerr << "RequestPUT content buffer: |" << this->_context.buffer << "|" << std::endl;

		// Read chunk
		if (static_cast<int32_t>(this->_context.buffer.size()) > this->_contentLength) {
			this->_file.write(this->_context.buffer.c_str(), this->_contentLength);
			this->_context.buffer.erase(0, this->_contentLength);
			this->_contentLength = 0;
		} else if (static_cast<int32_t>(this->_context.buffer.size()) <= this->_contentLength) {
			this->_file.write(this->_context.buffer.c_str(), this->_context.buffer.size());
			this->_contentLength -= this->_context.buffer.size();
			this->_context.buffer.clear();
		}
	}
	return (REQ_CONTINUE);
}

/* ************************************************************************** */

void RequestPUT::processing(void) {
	// Check headers
	if (REQ_CONTINUE != this->_checkHeaders()) {
		return;
	}

	// Check path
	if (this->_path.isDirFormat()) {
		this->_context.response.setStatusCode(STATUS_CONFLICT);
		return;
	}
	Path parent = this->_path.dir();
	if (0 != this->_path.access(F_OK)) {
		if (0 != this->_path.access(W_OK)) {
			this->_context.response.setStatusCode(STATUS_FORBIDDEN);
		} else {
			this->_openFile();
		}
		return ;
	}
	if (0 != parent.access(F_OK)) {
		this->_context.response.setStatusCode(STATUS_NOT_FOUND);
	} else if (0 != parent.stat()) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
	} else if (!parent.isDir()) {
		this->_context.response.setStatusCode(STATUS_CONFLICT);
	} else if (0 != parent.access(W_OK)) {
		this->_context.response.setStatusCode(STATUS_FORBIDDEN);
	} else {
		this->_openFile();
	}
}

error_t RequestPUT::workIn(void) {
	// std::cerr << "RequestPUT workIn" << std::endl;
	if (this->_chunked) {
		return (this->_readChunked());
	} else {
		return (this->_readContent());
	}
	return (REQ_DONE);
}

ARequest *RequestPUT::clone(void) const {
	// std::cerr << "RequestPUT clone" << std::endl;
	return (new RequestPUT(*this));
}

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */

/* OTHERS *********************************************************************/

ARequest *createRequestPUT(RequestContext_t &context) {
	// std::cerr << "createRequestPUT" << std::endl;
	return (new RequestPUT(context));
}

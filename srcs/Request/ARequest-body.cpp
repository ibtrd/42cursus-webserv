#include "ARequest.hpp"

void ARequest::_saveFile(void) {
	std::cerr << "ARequest _saveFile" << std::endl;
	throw std::logic_error("ARequest::_saveFile should not be called");
}

error_t ARequest::_writeChunk(void) {
	std::cerr << "ARequest _writeChunk" << std::endl;
	throw std::logic_error("ARequest::_writeChunk should not be called");
	return (REQ_ERROR);
}

error_t ARequest::_readContent(void) {
	if (this->_contentLength - static_cast<int32_t>(this->_context.buffer.size()) >= 0) {
		if (this->_writeChunk() == REQ_ERROR) {
			return (REQ_ERROR);
		}
		this->_contentLength -= this->_context.buffer.size();
		this->_context.buffer.clear();
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

error_t ARequest::_readChunked(void) {
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

		// Read chunk
		if (static_cast<int32_t>(this->_context.buffer.size()) > this->_contentLength) {
			if (this->_writeChunk() == REQ_ERROR) {
				return (REQ_ERROR);
			}
			this->_context.buffer.erase(0, this->_contentLength);
			this->_contentLength = 0;
		} else if (static_cast<int32_t>(this->_context.buffer.size()) <= this->_contentLength) {
			if (this->_writeChunk() == REQ_ERROR) {
				return (REQ_ERROR);
			}
			this->_contentLength -= this->_context.buffer.size();
			this->_context.buffer.clear();
		}
	}
	return (REQ_CONTINUE);
}

#include <ctime>
#include <iostream>

#include "RequestGET.hpp"
#include "ft.hpp"
#include "webservHTML.hpp"

#define KB 1024L
#define MB 1048576L
#define GB 1073741824L

static std::string bytesToUnit(long bytes) {
	if (bytes >= GB) {
		return ft::numToStr(bytes / GB) + " GB";
	} else if (bytes >= MB) {
		return ft::numToStr(bytes / MB) + " MB";
	} else if (bytes >= KB) {
		return ft::numToStr(bytes / KB) + " KB";
	} else {
		return ft::numToStr(bytes) + " B";
	}
}

error_t RequestGET::_readDir(void) {
	struct dirent *entry;
	std::string    buffer;
	size_t         entryCount = 0;

	errno = 0;
	while (entryCount < REQ_DIR_BUFFER_SIZE && (entry = readdir(this->_dir)) != NULL) {
		if (!std::strcmp(entry->d_name, ".")) {
			continue;
		}

		char nameBuffer[32]  = {0};
		char timeBuffer[128] = {0};

		if (strlen(entry->d_name) > 23) {
			std::strncpy(nameBuffer, entry->d_name, 20);
			std::strncpy(nameBuffer + 20, "...", 4);
			nameBuffer[24] = '\0';
		} else {
			std::strcpy(nameBuffer, entry->d_name);
		}

		Path localPath(this->_path.string() + entry->d_name);

		error_t err = localPath.stat();
		if (err == -1) {
			timeBuffer[0] = '\0';
		} else {
			std::strftime(timeBuffer, sizeof(timeBuffer), "%F %R",
			              std::localtime(&localPath.mTime()));
		}

		if (-1 == err) {
			buffer += HTMLWEIRD(
			    this->_context.headers[HEADER_HOST] + this->_context.target + entry->d_name,
			    nameBuffer);
		} else if (localPath.isDir()) {
			buffer +=
			    HTMLDIR(this->_context.headers[HEADER_HOST] + this->_context.target + entry->d_name,
			            nameBuffer, timeBuffer);
		} else if (localPath.isFile()) {
			buffer += HTMLFILE(
			    this->_context.headers[HEADER_HOST] + this->_context.target + entry->d_name,
			    nameBuffer, timeBuffer, (-1 != err ? " " + bytesToUnit(localPath.size()) : ""));
		} else {
			buffer += HTMLOTHER(
			    this->_context.headers[HEADER_HOST] + this->_context.target + entry->d_name,
			    nameBuffer, timeBuffer);
		}

		entryCount++;
		errno = 0;
	}
	if (errno != 0) {
		std::cerr << "readdir: " << strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return (REQ_ERROR);
	}
	this->_context.response.addBody(buffer);
	if (entry == NULL) {
		this->_context.response.addBody(HTMLFOOTER);
		SET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);
	}
	this->_context.responseBuffer += this->_context.response.body();
	this->_context.response.clearBody();
	return (REQ_CONTINUE);
}

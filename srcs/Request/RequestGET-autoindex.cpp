#include <iostream>

#include "RequestGET.hpp"
#include "ft.hpp"

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

error_t RequestGET::_readDir(void)
{
	struct dirent	*entry;
	std::string		buffer;
	size_t			entryCount = 0;

	errno = 0;
	while (entryCount < REQ_DIR_BUFFER_SIZE && (entry = readdir(this->_dir)) != NULL)
	{
		if (!std::strcmp(entry->d_name, ".")) continue;
		std::string fullPath = this->_context.target + entry->d_name;

		if (entry->d_type == DT_DIR) {
			buffer += "📁 <a href=\"http://" + this->_context.headers[HEADER_HOST] + this->_context.target + entry->d_name + "\">" + entry->d_name + "</a><br>";
		} else if (entry->d_type == DT_REG) {
			struct stat fileStat;
			std::string localPath = this->_path.string() + entry->d_name;
			error_t err = stat(localPath.c_str(), &fileStat);
			buffer += "📄 <a href=\"http://" + this->_context.headers[HEADER_HOST] + this->_context.target + entry->d_name + "\">" + entry->d_name + "</a>" + (-1 != err ? " " + bytesToUnit(fileStat.st_size) : "") + "<br>";
		} else {
			buffer += "❓ <a href=\"http://" + this->_context.headers[HEADER_HOST] + this->_context.target + entry->d_name + "\">" + entry->d_name + "</a><br>";
		}
		entryCount++;
		errno = 0;
	}
	if (errno != 0)
	{
		std::cerr << "readdir: " << strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return (REQ_ERROR);
	}
	this->_context.response.addBody(buffer);
	if (entry == NULL)
	{
		this->_context.response.addBody("</body></html>");
		SET_REQ_PROCESS_OUT_COMPLETE(this->_context.requestState);
	}
	this->_context.responseBuffer += this->_context.response.body();
	this->_context.response.clearBody();
	return (REQ_CONTINUE);
}

#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>

#include "RequestGET.hpp"
#include "ft.hpp"
#include "Client.hpp"
#include "Server.hpp"

/* CONSTRUCTORS ************************************************************* */

RequestGET::RequestGET(RequestContext_t &context) : ARequest(context), _dir(NULL)
{
	// std::cerr << "RequestGET created" << std::endl;
}

RequestGET::RequestGET(const RequestGET &other) : ARequest(other), _dir(NULL)
{
	// std::cerr << "RequestGET copy" << std::endl;
	*this = other;
}

RequestGET::~RequestGET(void)
{
	// std::cerr << "RequestGET destroyed" << std::endl;
	if (this->_file.is_open())
		this->_file.close();
	if (this->_dir != NULL)
		closedir(this->_dir);
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestGET	&RequestGET::operator=(const RequestGET &other)
{
	// std::cerr << "RequestGET assign" << std::endl;
	(void)other;
	return (*this);
}

/* ************************************************************************** */

void RequestGET::_openFile(void)
{
	// std::cerr << "RequestGET _openFile" << std::endl;
	this->_file.open(this->_path.string().c_str(), std::ios::in | std::ios::binary);
	if (!this->_file.is_open()) {
		std::cerr << "open: " << strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return ;
	}
	this->_context.response.setStatusCode(STATUS_OK);
	this->_context.response.setHeader(HEADER_CONTENT_TYPE, this->_context.server.getMimeType(this->_path.extension()));
	this->_file.seekg(0, std::ios::end);
	this->_context.response.setHeader(HEADER_CONTENT_LENGTH, ft::numToStr(this->_file.tellg()));
	this->_file.seekg(0, std::ios::beg);
}

void RequestGET::_openDir(void)
{
	// std::cerr << "RequestGET _openDir" << std::endl;
	this->_dir = opendir(this->_path.string().c_str());
	if (this->_dir == NULL) {
		std::cerr << "opendir: " << strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return ;
	}
	this->_context.response.setStatusCode(STATUS_OK);
	this->_context.response.setHeader(HEADER_CONTENT_TYPE, "text/html");
	// this->_context.response.setHeader(HEADER_TRANSFER_ENCODING, "chunked");

	std::string body;
	body = "<html><head><title>Index of " + this->_context.target + "</title></head><body><h1>Index of " + this->_context.target + "</h1><hr><pre>";
	this->_context.response.setBody(body);
}

error_t RequestGET::_readFile(void)
{
	char	buffer[REQ_BUFFER_SIZE];

	this->_file.read(buffer, REQ_BUFFER_SIZE);
	ssize_t	bytes = this->_file.gcount();
	if (bytes == 0)
	{
		SET_REQ_PROCESS_OUT_COMPLETE(this->_context.requestState);
		return (REQ_CONTINUE);
	}

	this->_context.responseBuffer.append(buffer, bytes);
	return (REQ_CONTINUE);
}

error_t RequestGET::_readDir(void)
{
	struct dirent	*entry;
	std::string		buffer;
	size_t			entryCount = 0;

	errno = 0;
	while (entryCount < REQ_DIR_BUFFER_SIZE && (entry = readdir(this->_dir)) != NULL)
	{
		buffer += "<a href=\"http://" + this->_context.headers[HEADER_HOST] + this->_context.target + entry->d_name + "\">" + entry->d_name + "</a><br>";
		entryCount++;
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
		this->_context.response.addBody("</pre><hr></body></html>");
		SET_REQ_PROCESS_OUT_COMPLETE(this->_context.requestState);
	}
	this->_context.responseBuffer += this->_context.response.body();
	this->_context.response.clearBody();
	return (REQ_CONTINUE);
}

/* ************************************************************************** */

error_t	RequestGET::parse(void)
{
	// std::cerr << "RequestGET parse" << std::endl;
	SET_REQ_READ_BODY_COMPLETE(this->_context.requestState);
	return (REQ_DONE);
}

error_t	RequestGET::processIn(void)
{
	// std::cerr << "RequestGET processIn" << std::endl;

	if (!this->_path.exists()) {
		this->_context.response.setStatusCode(STATUS_NOT_FOUND);
		SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	if (this->_path.hasPermission(R_OK) == false) {
		this->_context.response.setStatusCode(STATUS_FORBIDDEN);
		SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	if (this->_path.isFile()) {
		this->_openFile();
		SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	if (!this->_path.isDir()) {
		this->_context.response.setStatusCode(STATUS_CONFLICT);
		SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	if (!this->_path.isDirFormat()) {
		this->_context.response.setStatusCode(STATUS_MOVED_PERMANENTLY);
		this->_context.response.setHeader(HEADER_LOCATION, this->_context.target + '/');
		SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	/* to implement: index */
	// iterate over index files (exists)
	// if found:
	//  if not readable -> 403
	//  if not regular file -> 409
	//  open file
	//  return

	if (this->_context.ruleBlock->isDirListing()) {
		this->_openDir();
		SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	this->_context.response.setStatusCode(STATUS_FORBIDDEN);
	SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
	return (REQ_DONE);
}

error_t	RequestGET::processOut(void)
{
	// std::cerr << "RequestGET processOut" << std::endl;
	if (this->_context.response.statusCode() != STATUS_OK)
	{
		SET_REQ_PROCESS_OUT_COMPLETE(this->_context.requestState);
		return (REQ_CONTINUE);
	}

	if (this->_path.isFileFormat())
		return (this->_readFile());

	if (this->_path.isDirFormat())
		return (this->_readDir());

	return (REQ_ERROR);
}

ARequest	*RequestGET::clone(void) const
{
	// std::cerr << "RequestGET clone" << std::endl;
	return (new RequestGET(*this));
}

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */

/* OTHERS *********************************************************************/

ARequest	*createRequestGET(RequestContext_t &context)
{
	// std::cerr << "createRequestGET" << std::endl;
	return (new RequestGET(context));
}

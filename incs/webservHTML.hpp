#ifndef WEBSERVHTML_HPP
#define WEBSERVHTML_HPP

#define INDEXOF(path)   \
	"\
<!DOCTYPE html>\
<html lang=\"en\">\
\
<head>\
	<meta charset=\"UTF-8\">\
	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
	<title>Index of " + \
	    path +          \
	    "</title>\
	<style>\
		body {\
			font-family: \"Noto Sans Mono\", sans-serif;\
			max-width: 100vw;\
			margin: 0 auto;\
			padding: 20px;\
		}\
		h1 {\
			text-align: left;\
			color: #333;\
			padding-bottom: 10px;\
		}\
		table {\
			border-collapse: collapse;\
		}\
		th {\
			border: none;\
			padding: 6px;\
			text-align: left;\
			white-space: nowrap;\
		}\
		td {\
			border: none;\
			padding: 6px;\
			text-align: left;\
			font-weight: normal;\
			white-space: nowrap;\
		}\
		.file-name {\
			display: inline-block;\
			/* width: 200px; */\
			/* padding-right: 10px; */\
		}\
		.size-column {\
			text-align: right;\
		}\
	</style>\
</head>\
<body>\
	<h1>Index of " +    \
	    path +          \
	    "</h1>\
	<table>\
		<tr>\
			<th></th> <th><span>Name</span></a></th> <th>Last modified</th> <th><span>Size</span></th>\
		</tr>\
		<tr>\
			<td colspan=\"5\"><hr></td>\
		</tr>"

#define HTMLFILE(path, name, time, size)                                                      \
	"<tr><td>📄</td> <td><a href=\"http://" + path + "\"><span class=\"file-name\">" + name + \
	    "</span></a></td> <td>" + time + "</td> <td class=\"size-column\"><span>" + size +    \
	    "</span></td></tr>"
#define HTMLDIR(path, name, time)                                                             \
	"<tr><td>📁</td> <td><a href=\"http://" + path + "\"><span class=\"file-name\">" + name + \
	    "</span></a></td> <td>" + time +                                                      \
	    "</td> <td class=\"size-column\"><span>-</span></td></tr>"
#define HTMLOTHER(path, name, time)                                                           \
	"<tr><td>❓</td> <td><a href=\"http://" + path + "\"><span class=\"file-name\">" + name + \
	    "</span></a></td> <td>" + time +                                                      \
	    "</td> <td class=\"size-column\"><span>-</span></td></tr>"
#define HTMLWEIRD(path, name)                                                                 \
	"<tr><td>❌</td> <td><a href=\"http://" + path + "\"><span class=\"file-name\">" + name + \
	    "</span></a></td> <td></td> <td class=\"size-column\"><span>-</span></td></tr>"

#define HTMLFOOTER "</table></body></html>"

#define HTMLERROR(code, message)       \
	"\
<!DOCTYPE html>\
<html lang=\"en\">\
<head>\
	<style>\
		* {\
			transition: all 0.6s;\
		}\
		html {\
			background-color: #242424;\
			height: 100%;\
		}\
		body {\
			font-family: 'Lato', sans-serif;\
			color: #888;\
			margin: 0;\
		}\
		#main {\
			display: table;\
			width: 100%;\
			height: 100vh;\
			text-align: center;\
		}\
		.fof {\
			display: table-cell;\
			vertical-align: middle;\
		}\
		.fof h1 {\
			font-size: 50px;\
			display: inline-block;\
			padding-right: 12px;\
			animation: type .5s alternate infinite;\
		}\
		@keyframes type {\
			from {\
				box-shadow: inset -3px 0px 0px #a8a8a8;\
			}\
			to {\
				box-shadow: inset -3px 0px 0px transparent;\
			}\
		}\
		.footer {\
			position: fixed;\
			left: 0;\
			bottom: 10px;\
			width: 100%;\
			text-align: center;\
			font-size: 12px;\
		}\
		.footer a {\
			color: #c2c2c2;\
			text-decoration: none;\
		}\
		.footer a:hover {\
			color: #067dcc;\
		}\
	</style>\
</head>\
<body>\
	<div id=\"main\">\
		<div class=\"fof\">\
			<h1>Error " +              \
	    code + "</h1><br>" + message + \
	    "<br>\
		</div>\
	</div>\
	<div class=\"footer\">webserv |\
		<a href=\"https://profile.intra.42.fr/users/kchillon\">@kchillon</a>\
		<a href=\"https://profile.intra.42.fr/users/ibertran\">@ibertran</a>\
	</div>\
</body>\
</html>"

// Client Errors
#define ERROR400MSG "The server could not understand your request. Please check your input."
#define ERROR401MSG "Authentication required. Please log in to access this resource."
#define ERROR403MSG "Access denied. You don't have permission to access this resource."
#define ERROR404MSG "It looks like the page you were looking for doesn't exist."
#define ERROR405MSG "The HTTP method you used is not allowed for this resource."
#define ERROR408MSG "The server timed out waiting for your request. Please try again."
#define ERROR409MSG \
	"A conflict occurred. Your request could not be processed due to conflicting data."
#define ERROR411MSG "The server requires a valid Content-Length header in your request."
#define ERROR413MSG "The request payload is too large. Please reduce its size and try again."
#define ERROR414MSG "The request URI is too long. Please shorten the URL and try again."
#define ERROR415MSG "The media type in your request is unsupported by the server."
#define ERROR418MSG "I'm a teapot. This server refuses to brew coffee with a teapot."
#define ERROR429MSG "Too many requests. Please slow down and try again later."

// Server Errors
#define ERROR500MSG "The server encountered an internal error. Please try again later."
#define ERROR501MSG "The server does not support the functionality required to fulfill the request."
#define ERROR503MSG "The service is currently unavailable. Please try again later."
#define ERROR504MSG "The server timed out waiting for a response from the upstream server."
#define ERROR505MSG "The HTTP version used in your request is not supported."

#endif /* ******************************************************************* */

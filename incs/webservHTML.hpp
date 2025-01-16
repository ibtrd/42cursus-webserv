#ifndef WEBSERVHTML_HPP
# define WEBSERVHTML_HPP

# define INDEXOF(path) "\
<!DOCTYPE html>\
<html lang=\"en\">\
\
<head>\
	<meta charset=\"UTF-8\">\
	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
	<title>Index of " + path + "</title>\
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
	<h1>Index of " + path + "</h1>\
	<table>\
		<tr>\
			<th></th> <th><span>Name</span></a></th> <th>Last modified</th> <th><span>Size</span></th>\
		</tr>\
		<tr>\
			<td colspan=\"5\"><hr></td>\
		</tr>"

# define HTMLFILE(path, name, time, size) "<tr><td>📄</td> <td><a href=\"http://" + path + "\"><span class=\"file-name\">" + name + "</span></a></td> <td>" + time + "</td> <td class=\"size-column\"><span>" + size + "</span></td></tr>"
# define HTMLDIR(path, name, time) "<tr><td>📁</td> <td><a href=\"http://" + path + "\"><span class=\"file-name\">" + name + "</span></a></td> <td>" + time + "</td> <td class=\"size-column\"><span>-</span></td></tr>"
# define HTMLOTHER(path, name, time) "<tr><td>❓</td> <td><a href=\"http://" + path + "\"><span class=\"file-name\">" + name + "</span></a></td> <td>" + time + "</td> <td class=\"size-column\"><span>-</span></td></tr>"
# define HTMLWEIRD(path, name) "<tr><td>❌</td> <td><a href=\"http://" + path + "\"><span class=\"file-name\">" + name + "</span></a></td> <td></td> <td class=\"size-column\"><span>-</span></td></tr>"

# define HTMLFOOTER "</table></body></html>"

# define HTMLERROR(code, message) "\
<!DOCTYPE html>\
<html lang=\"en\">\
<head>\
    <meta charset=\"UTF-8\">\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
    <title>Error " + code + "</title>\
	<style>\
		:root {\
			font-size: 20px;\
			font-family: 'IBM Plex Mono', monospace;\
			line-height: 1.5;\
			color: rgba(255, 255, 255, 0.25);\
		}\
		body {\
			height: 100vh;\
			display: flex;\
			flex-direction: column;\
			justify-content: center;\
			align-items: center;\
			text-align: center;\
			background: #333333;\
			margin: 0;\
		}\
		a {\
			color: white;\
			display: inline;\
		}\
		#error {\
			margin-bottom: 1rem;\
			font-size: 2rem;\
			font-weight: 500;\
			text-transform: uppercase;\
			letter-spacing: 0.075em;\
			color: #C94D4D;\
			animation: pulse 4s infinite alternate;\
			position: relative;\
		}\
		#error::before {\
			content: '';\
			width: 0.75rem;\
			height: 50vh;\
			margin-bottom: 0.75em;\
			position: absolute;\
			left: 50%;\
			bottom: 100%;\
			transform: translateX(-50%);\
			background: linear-gradient(to bottom, rgba(255, 255, 255, 0.1) 60%, transparent 100%);\
		}\
		@keyframes pulse {\
			from {\
				opacity: 0.5;\
			}\
			50% {\
				opacity: 0.5;\
			}\
		}\
		#desc {\
			margin: 2em 0 1em;\
		}\
		.error-num {\
			position: relative;\
			font-size: 10rem;\
			font-family: 'Oswald', sans-serif;\
			letter-spacing: -0.01em;\
			color: white;\
			animation: colorSplit 1.25s steps(2, end) infinite;\
		}\
		@keyframes colorSplit {\
			25% {\
				text-shadow: -0.02em 0 0 #ED008C, 0.025em 0 0 #0087EF;\
			}\
			75% {\
				text-shadow: -0.035em 0 0 #ED008C, 0.04em 0 0 #0087EF;\
			}\
		}\
		.error-num__clip {\
			position: absolute;\
			top: 0;\
			left: -2px;\
			z-index: 10;\
			color: #333;\
			overflow: visible;\
			clip-path: polygon(0% 0%, 100% 0, 100% 25%, 0 25%, 0 30%, 100% 30%, 100% 50%, 0 50%, 0 60%, 100% 60%, 100% 65%, 0 65%, 0 80%, 100% 80%, 100% 85%, 0 85%, 0% 0%);\
			animation: glitch 1s steps(2, start) infinite;\
		}\
		.error-num__clip::before {\
			content: '" + code + "';\
			position: absolute;\
			top: 0;\
			left: 0.05em;\
			z-index: 9;\
			color: white;\
			clip-path: polygon(0% 0%, 100% 0, 100% 26%, 0 26%, 0 29%, 100% 29%, 100% 51%, 0 51%, 0 59%, 100% 59%, 100% 66%, 0 66%, 0 79%, 100% 79%, 100% 86%, 0 86%, 0% 0%);\
		}\
		@keyframes glitch {\
			30% {\
				left: 0;\
			}\
			to {\
				left: 0;\
			}\
		}\
	</style>\
</head>\
<body>\
    <div id=\"error\">Error</div>\
    <div class=\"error-num\">\
        " + code + "\
        <div class=\"error-num__clip\">" + code + "</div>\
    </div>\
    <p id=\"desc\">Uh oh, there seems to be a problem.<br>" + message + "</p>\
    <p>Let me help you find <a href=\"https://youtu.be/dQw4w9WgXcQ\" target=\"_blank\">a way out</a></p>\
</body>\
</html>"

// Client Errors
#define ERROR400MSG "The server could not understand your request. Please check your input."
#define ERROR401MSG "Authentication required. Please log in to access this resource."
#define ERROR403MSG "Access denied. You don't have permission to access this resource."
#define ERROR404MSG "It looks like the page you were looking for doesn't exist."
#define ERROR405MSG "The HTTP method you used is not allowed for this resource."
#define ERROR408MSG "The server timed out waiting for your request. Please try again."
#define ERROR409MSG "A conflict occurred. Your request could not be processed due to conflicting data."
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
#define ERROR505MSG "The HTTP version used in your request is not supported."


#endif /* ******************************************************************* */

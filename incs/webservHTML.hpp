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
# define HTMLERROR(path, name) "<tr><td>❌</td> <td><a href=\"http://" + path + "\"><span class=\"file-name\">" + name + "</span></a></td> <td></td> <td class=\"size-column\"><span>-</span></td></tr>"

#endif /* ******************************************************************* */

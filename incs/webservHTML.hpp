#ifndef WEBSERVHTML_HPP
# define WEBSERVHTML_HPP

# define INDEXOF(path) "\
<!DOCTYPE html> \
<html lang=\"en\"> \
<head> \
	<meta charset=\"UTF-8\"> \
	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> \
	<title>Index of " + path + "</title> \
	<style> \
		body { \
			font-family: Arial, sans-serif; \
			max-width: 100vw; \
			margin: 0 auto; \
			padding: 20px; \
		} \
		h1 { \
			text-align: left; \
			color: #333; \
			border-bottom: 2px solid #0066cc; \
			padding-bottom: 10px; \
		} \
	</style> \
</head> \
<body> \
	<h1>Index of " + path + "</h1>"

#endif /* ******************************************************************* */

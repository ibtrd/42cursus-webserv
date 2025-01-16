#ifndef METHOD_HPP
# define METHOD_HPP

# include <vector>
# include <string>

typedef enum {
	GET,
	POST,
	DELETE,
	PUT,
	INVAL_METHOD
} method_t;


class Method {
public:
	Method(void);
	Method(const Method &other);
	Method(const std::string &str);
	~Method(void);

	Method	&operator=(const Method &other);

	bool	operator==(const Method &other);
	bool	operator==(const method_t method);
	bool	operator!=(const Method &other);
	bool	operator!=(const method_t method);

	method_t			index(void) const;
	const std::string	&string(void) const;

	bool	isValid(void) const;

	static const std::vector<std::string>	methods;

private:
	method_t	_index; 

	static std::vector<std::string>	_initMethods(void);
};

#endif /* ******************************************************************* */

// GET /favicon.ico HTTP/1.1
// Accept: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8
// Accept-Encoding: gzip, deflate, br, zstd
// Accept-Language: en-US,en;q=0.9
// Connection: keep-alive
// Host: 127.0.0.1:8080
// Referer: http://127.0.0.1:8080/
// Sec-Fetch-Dest: image
// Sec-Fetch-Mode: no-cors
// Sec-Fetch-Site: same-origin
// User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36
// sec-ch-ua: "Google Chrome";v="131", "Chromium";v="131", "Not_A Brand";v="24"
// sec-ch-ua-mobile: ?0
// sec-ch-ua-platform: "Linux"


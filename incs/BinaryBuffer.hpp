#ifndef BINARYBUFFER_HPP
# define BINARYBUFFER_HPP

# include <vector>
# include <stdint.h>
# include <iostream>

class BinaryBuffer {
private:
	std::vector<uint8_t> _buffer;

public:
	BinaryBuffer(void);
	BinaryBuffer(const BinaryBuffer &other);
	BinaryBuffer(const std::vector<uint8_t> &buffer);
	BinaryBuffer(const uint8_t *buffer, size_t size);
	BinaryBuffer(const char *c_string);

	~BinaryBuffer(void);

	BinaryBuffer &operator=(const BinaryBuffer &other);

	void append(const BinaryBuffer &other);
	void append(const std::vector<uint8_t> &buffer);
	void append(const uint8_t *buffer, size_t size);
	void append(const char *c_string);

	void clear(void);

	size_t find(const BinaryBuffer &other) const;
	size_t find(const std::vector<uint8_t> &buffer) const;
	size_t find(const uint8_t *buffer, size_t size) const;
	size_t find(const char *c_string) const;

	size_t rfind(const BinaryBuffer &other, size_t pos) const;
	size_t rfind(const std::vector<uint8_t> &buffer, size_t pos) const;
	size_t rfind(const uint8_t *buffer, size_t size, size_t pos) const;	// Like std::string::rfind but with uint8_t (uint8_t *buffer, size_t pos). pos is the position to start the search from.
	size_t rfind(const char *c_string, size_t pos) const;

	int compare(const BinaryBuffer &other) const;
	int compare(const std::vector<uint8_t> &buffer) const;
	int compare(const uint8_t *buffer, size_t size) const;
	int compare(const char *c_string) const;

	const std::vector<uint8_t> &buffer(void) const;
	size_t                     size(void) const;
	const uint8_t              *data(void) const;
	const char                 *c_str(void) const;
	BinaryBuffer               subbb(size_t pos, size_t len) const;
	std::string                substr(size_t pos, size_t len) const;
	BinaryBuffer               &erase(size_t pos, size_t len);
	bool                       empty(void) const;

	bool operator==(const BinaryBuffer &other) const;
	bool operator!=(const BinaryBuffer &other) const;

	bool operator==(const std::vector<uint8_t> &buffer) const;
	bool operator!=(const std::vector<uint8_t> &buffer) const;

	bool operator==(const uint8_t *buffer) const;
	bool operator!=(const uint8_t *buffer) const;

	bool operator==(const char *c_string) const;
	bool operator!=(const char *c_string) const;

	BinaryBuffer &operator+=(const BinaryBuffer &other);
	BinaryBuffer &operator+=(const std::vector<uint8_t> &buffer);
	BinaryBuffer &operator+=(const char *c_string);

	BinaryBuffer operator+(const BinaryBuffer &other) const;
	BinaryBuffer operator+(const std::vector<uint8_t> &buffer) const;
	BinaryBuffer operator+(const char *c_string) const;

	uint8_t &operator[](size_t pos);
	const uint8_t &operator[](size_t pos) const;

	friend std::ostream &operator<<(std::ostream &os, const BinaryBuffer &buffer);
};

#endif

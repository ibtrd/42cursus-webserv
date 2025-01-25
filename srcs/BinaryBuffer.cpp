#include "BinaryBuffer.hpp"
#include <cstring>
#include <algorithm>

BinaryBuffer::BinaryBuffer(void) : _buffer() {}

BinaryBuffer::BinaryBuffer(const BinaryBuffer &other) : _buffer(other._buffer) {}

BinaryBuffer::BinaryBuffer(const std::vector<uint8_t> &buffer) : _buffer(buffer) {}

BinaryBuffer::BinaryBuffer(const uint8_t *buffer, size_t size) : _buffer(buffer, buffer + size) {}

BinaryBuffer::BinaryBuffer(const char *c_string) : _buffer(c_string, c_string + strlen(c_string)) {}

BinaryBuffer::~BinaryBuffer(void) {}

BinaryBuffer &BinaryBuffer::operator=(const BinaryBuffer &other) {
	if (this == &other) {
		return (*this);
	}
	this->_buffer = other._buffer;
	return (*this);
}

BinaryBuffer &BinaryBuffer::operator=(const std::vector<uint8_t> &buffer) {
	this->_buffer = buffer;
	return (*this);
}

BinaryBuffer &BinaryBuffer::operator=(const char *c_string) {
	this->_buffer.assign(c_string, c_string + strlen(c_string));
	return (*this);
}

BinaryBuffer &BinaryBuffer::operator=(const std::string &str) {
	this->_buffer.assign(str.begin(), str.end());
	return (*this);
}

void BinaryBuffer::append(const BinaryBuffer &other) {
	this->_buffer.insert(this->_buffer.end(), other._buffer.begin(), other._buffer.end());
}

void BinaryBuffer::append(const std::vector<uint8_t> &buffer) {
	this->_buffer.insert(this->_buffer.end(), buffer.begin(), buffer.end());
}

void BinaryBuffer::append(const uint8_t *buffer, size_t size) {
	this->_buffer.insert(this->_buffer.end(), buffer, buffer + size);
}

void BinaryBuffer::append(const char *c_string) {
	this->_buffer.insert(this->_buffer.end(), c_string, c_string + strlen(c_string));
}

void BinaryBuffer::clear(void) { this->_buffer.clear(); }

size_t BinaryBuffer::find(const BinaryBuffer &other) const {
	return (this->find(other._buffer));
}

size_t BinaryBuffer::find(const std::vector<uint8_t> &buffer) const {
	return (this->find(buffer.data(), buffer.size()));
}

size_t BinaryBuffer::find(const uint8_t *buffer, size_t size) const {
	const uint8_t *pos = std::search(this->_buffer.data(), this->_buffer.data() + this->_buffer.size(), buffer, buffer + size);
	if (pos == this->_buffer.data() + this->_buffer.size()) {
		return (std::string::npos);
	}
	return (pos - this->_buffer.data());
}

size_t BinaryBuffer::find(const char *c_string) const {
	return (this->find(reinterpret_cast<const uint8_t *>(c_string), strlen(c_string)));
}

size_t BinaryBuffer::rfind(const BinaryBuffer &other, size_t pos) const {
	return (this->rfind(other._buffer, pos));
}

size_t BinaryBuffer::rfind(const std::vector<uint8_t> &buffer, size_t pos) const {
	return (this->rfind(buffer.data(), buffer.size(), pos));
}

size_t BinaryBuffer::rfind(const uint8_t *buffer, size_t size, size_t pos) const {
	// Return npos for invalid size or buffer size
	if (size == 0 || size > this->_buffer.size()) {
		return std::string::npos;
	}

	// Adjust `pos` to be the last possible starting point
	if (pos >= this->_buffer.size()) {
		pos = this->_buffer.size() - 1;
	}

	// Search backwards
	for (size_t i = pos + 1; i-- > 0;) {
		if (size <= this->_buffer.size() - i && std::memcmp(buffer, this->_buffer.data() + i, size) == 0) {
			return i;
		}
	}
	return std::string::npos; // No match found
}


size_t BinaryBuffer::rfind(const char *c_string, size_t pos) const {
	return (this->rfind(reinterpret_cast<const uint8_t *>(c_string), strlen(c_string), pos));
}

int BinaryBuffer::compare(const BinaryBuffer &other) const {
	return (this->compare(other._buffer));
}

int BinaryBuffer::compare(const std::vector<uint8_t> &buffer) const {
	return (this->compare(buffer.data(), buffer.size()));
}

int BinaryBuffer::compare(const uint8_t *buffer, size_t size) const {
	return (std::memcmp(this->_buffer.data(), buffer, std::min(size, this->_buffer.size())));
}

int BinaryBuffer::compare(const char *c_string) const {
	return (this->compare(reinterpret_cast<const uint8_t *>(c_string), strlen(c_string)));
}

const std::vector<uint8_t> &BinaryBuffer::buffer(void) const { return (this->_buffer); }

size_t BinaryBuffer::size(void) const { return (this->_buffer.size()); }

const uint8_t *BinaryBuffer::data(void) const { return (this->_buffer.data()); }

const char *BinaryBuffer::c_str(void) const {
	if (this->_buffer.empty()) {
		return ("");
	}
	return (reinterpret_cast<const char *>(this->_buffer.data()));
}

BinaryBuffer BinaryBuffer::subbb(size_t pos, size_t len) const {
	if (pos >= this->_buffer.size()) {
		return (BinaryBuffer());
	}
	return (BinaryBuffer(this->_buffer.data() + pos, std::min(len, this->_buffer.size() - pos)));
}

std::string BinaryBuffer::substr(size_t pos, size_t len) const {
	if (pos >= this->_buffer.size()) {
		return (std::string());
	}
	return (std::string(reinterpret_cast<const char *>(this->_buffer.data() + pos), std::min(len, this->_buffer.size() - pos)));
}

BinaryBuffer &BinaryBuffer::erase(size_t pos, size_t len) {
	if (pos >= this->_buffer.size()) {
		return (*this);
	}
	this->_buffer.erase(this->_buffer.begin() + pos, this->_buffer.begin() + std::min(pos + len, this->_buffer.size()));
	return (*this);
}

bool BinaryBuffer::empty(void) const { return (this->_buffer.empty()); }

bool BinaryBuffer::operator==(const BinaryBuffer &other) const {
	return (this->compare(other) == 0);
}

bool BinaryBuffer::operator!=(const BinaryBuffer &other) const {
	return (this->compare(other) != 0);
}

bool BinaryBuffer::operator==(const std::vector<uint8_t> &buffer) const {
	return (this->compare(buffer) == 0);
}

bool BinaryBuffer::operator!=(const std::vector<uint8_t> &buffer) const {
	return (this->compare(buffer) != 0);
}

bool BinaryBuffer::operator==(const uint8_t *buffer) const {
	return (this->compare(buffer, std::strlen(reinterpret_cast<const char *>(buffer))) == 0);
}

bool BinaryBuffer::operator!=(const uint8_t *buffer) const {
	return (this->compare(buffer, std::strlen(reinterpret_cast<const char *>(buffer))) != 0);
}

bool BinaryBuffer::operator==(const char *c_string) const {
	return (this->compare(reinterpret_cast<const uint8_t *>(c_string), std::strlen(c_string)) == 0);
}

bool BinaryBuffer::operator!=(const char *c_string) const {
	return (this->compare(reinterpret_cast<const uint8_t *>(c_string), std::strlen(c_string)) != 0);
}

BinaryBuffer &BinaryBuffer::operator+=(const BinaryBuffer &other) {
	this->append(other);
	return (*this);
}

BinaryBuffer &BinaryBuffer::operator+=(const std::vector<uint8_t> &buffer) {
	this->append(buffer);
	return (*this);
}

BinaryBuffer &BinaryBuffer::operator+=(const char *c_string) {
	this->append(c_string);
	return (*this);
}

BinaryBuffer BinaryBuffer::operator+(const BinaryBuffer &other) const {
	BinaryBuffer result(*this);
	result += other;
	return (result);
}

BinaryBuffer BinaryBuffer::operator+(const std::vector<uint8_t> &buffer) const {
	BinaryBuffer result(*this);
	result += buffer;
	return (result);
}

BinaryBuffer BinaryBuffer::operator+(const char *c_string) const {
	BinaryBuffer result(*this);
	result += c_string;
	return (result);
}

uint8_t &BinaryBuffer::operator[](size_t pos) { return (this->_buffer[pos]); }

const uint8_t &BinaryBuffer::operator[](size_t pos) const { return (this->_buffer[pos]); }

std::ostream &operator<<(std::ostream &os, const BinaryBuffer &buffer) {
	for (size_t i = 0; i < buffer.size(); ++i) {
		os << buffer[i];
	}
	return (os);
}

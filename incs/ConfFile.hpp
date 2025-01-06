#ifndef CONFFILE_HPP
# define CONFFILE_HPP

# include <map>

# include <ConfToken.hpp>
# include <ServerBlock.hpp>

class ConfFile {
public:
	ConfFile(std::vector<ServerBlock> *blocks);
	~ConfFile(void);

	void	parse(void);

	// GETTERS
	const std::string 				&path(void) const;
	const std::vector<ConfToken>	&tokens(void) const;

	bool	eof(std::vector<ConfToken>::const_iterator &it) const;

	// SETTERS
	void	setPath(const std::string &path);

private:
	std::string					_path;
	std::vector<ConfToken>		_tokens;
	std::vector<ServerBlock>	*_blocks;

	void	_tokenize(const std::string &line, const uint32_t index);

	void	_serverDirective(std::vector<ConfToken>::const_iterator &token);

	void	_listenDirective(std::vector<ConfToken>::const_iterator &token, ServerBlock &server);
	void	_serverNameDirective(std::vector<ConfToken>::const_iterator &token, ServerBlock &server);
	void	_locationDirective(std::vector<ConfToken>::const_iterator &token, ServerBlock &server);

	void	_rootDirective(std::vector<ConfToken>::const_iterator &token, LocationBlock &location);
	void	_allowDirective(std::vector<ConfToken>::const_iterator &token, LocationBlock &location);
	void	_clientMaxBodySizeDirective(std::vector<ConfToken>::const_iterator &token, LocationBlock &location);
	void	_autoindexDirective(std::vector<ConfToken>::const_iterator &token, LocationBlock &location);

	uint32_t	_countArgs(const std::vector<ConfToken>::const_iterator &directive) const;

	std::string _unkwownDirective(const ConfToken &error) const;
	std::string _missingOpening(const ConfToken &error, const char c) const;
	std::string _invalidArgumentNumber(const ConfToken &error) const;
	std::string _unexpectedEOF(const ConfToken &error, const char c) const;
	std::string _unexpectedEOF(const ConfToken &error, const char c1, const char c2) const;
	std::string _unexpectedToken(const ConfToken &error) const;
	std::string _hostNotFound(const ConfToken &error, const ConfToken &host) const;
	std::string _invalidMethod(const ConfToken &method) const;
	std::string _invalidValue(const ConfToken &directive, const ConfToken &value) const;
	std::string _invalidValue(const ConfToken &directive, const ConfToken &inval, const std::string &expected1, const std::string &expected2) const;

	// STATICS
	static bool _isMetachar(int c);
	static bool _isComment(int c);

	typedef std::map<const std::string, void (ConfFile::*)(std::vector<ConfToken>::const_iterator &token)> directives;
	static const directives	_directives;
	static directives		_initializeDirectives(void);

	typedef std::map<const std::string, void (ConfFile::*)(std::vector<ConfToken>::const_iterator &tok, ServerBlock &block)> serverDirectives;
	static const serverDirectives	_serverDirectives;
	static serverDirectives			_initServerDirectives(void);

	typedef std::map<const std::string, void (ConfFile::*)(std::vector<ConfToken>::const_iterator &tok, LocationBlock &location)> locationDirectives;
	static const locationDirectives	_locationDirectives;
	static locationDirectives		_initLocationDirectives(void);
};

#endif /* ******************************************************************* */

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
	const std::string 			 &path(void) const;
	const std::vector<ConfToken> &tokens(void) const;

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


	std::string _unkwownDirective(const ConfToken &error);
	std::string _missingOpening(const ConfToken &error, const char c);
	std::string _invalidArgumentNumber(const ConfToken &error);
	std::string _unexpectedEOF(const ConfToken &error, const char c);
	std::string _unexpectedEOF(const ConfToken &error, const char c1, const char c2);
	std::string _unexpectedToken(const ConfToken &error);
	std::string _hostNotFound(const ConfToken &error, const ConfToken &host);
	std::string _invalidMethod(const ConfToken &method);

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

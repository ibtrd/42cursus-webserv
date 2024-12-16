#ifndef SERVERBLOCK_HPP
# define SERVERBLOCK_HPP

class ServerBlock {
public:
	ServerBlock(void);
	ServerBlock(const ServerBlock &other);

	~ServerBlock(void);

	ServerBlock	&operator=(const ServerBlock &other);
};

#endif /* ******************************************************************* */

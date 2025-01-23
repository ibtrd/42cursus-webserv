#ifndef ENV_HPP
#define ENV_HPP

#include <string>
#include <vector>

class Env {
public:
	Env(void);
	Env(const Env &other);
	~Env(void);

	Env &operator=(const Env &other);

	void   add(const std::string &key, const std::string &val);
	char **envp(void) const;

private:
	std::vector<std::string> _envars;

	friend std::ostream &operator<<(std::ostream &os, const Env &path);
};

#endif /* ******************************************************************* */

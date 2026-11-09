#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
    int			_fd;
    std::string	_buffer;
    bool		_disconnected;
	std::string	_nick;
	std::string	_username;
	std::string	_realname;
	bool		_hasPassword;
	bool		_registered;
	std::string	_sendBuffer;

public:
    Client(int fd);

    int					getFd() const;
    const std::string&	getBuffer() const;
    bool				isDisconnected() const;

    void				addData(const char *data, int bytes);
    void				disconnect();
	bool				getNextMessage(std::string& message);
	void				queueMessage(const std::string& message);
	const std::string&	getSendBuffer() const;
    void				removeSentData(size_t bytes);
	void				setPassword();
	bool				hasPassword();
	bool				isRegistered();
	void				registerClient();
	void				setNick(std::string nick);
	void				setUsername(std::string username);
	void				setRealname(std::string realname);
	std::string			getNick();
	std::string			getUsername();
	std::string			getRealname();
};

#endif
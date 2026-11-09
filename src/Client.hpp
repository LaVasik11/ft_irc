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
	std::string	_sendBuffer;

public:
    Client(int fd);

    int					getFd() const;
    const std::string&	getBuffer() const;
    bool				isDisconnected() const;

    void				addData(const char *data, int bytes);
    void				disconnect();
	bool				getNextMessage(std::string& message);
	void				setNick(std::string nick);
	std::string			getNick();
	void				queueMessage(const std::string& message);
	const std::string&	getSendBuffer() const;
    void				removeSentData(size_t bytes);
};

#endif
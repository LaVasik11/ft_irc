#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
    int			_fd;
    std::string	_buffer;
    bool		_disconnected;

public:
    Client(int fd);

    int					getFd() const;
    const std::string&	getBuffer() const;
    bool				isDisconnected() const;

    void				addData(const char *data, int bytes);
    void				disconnect();
	bool				getNextMessage(std::string& message);
};

#endif
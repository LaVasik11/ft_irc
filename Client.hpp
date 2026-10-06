#ifndef CLIENT_HPP
#define CLIENT_HPP


#include <poll.h>
#include <iostream>

class Client
{
	private:
		int			_fd;
		std::string	_buffer;

	public:
		Client(int fd) : _fd(fd) {};

		int				getFd() const;
		std::string&	getBuffer();
		void			addData(const char *data, int bytes);
		bool			getNextMessage(std::string& message);

};
#endif
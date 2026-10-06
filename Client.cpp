#include "Client.hpp"

int				Client::getFd() const
{
	return (_fd);
}
		
std::string&	Client::getBuffer()
{
	return (_buffer);
}

void			Client::addData(const char *data, int bytes)
{
	_buffer.append(data, bytes);
}

bool			Client::getNextMessage(std::string& message)
{
    size_t pos = _buffer.find("\r\n");

    if (pos == std::string::npos)
        return false;

    message = _buffer.substr(0, pos);
    _buffer.erase(0, pos + 2);

    return true;
}
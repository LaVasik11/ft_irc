#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _disconnected(false) {}

int Client::getFd() const
{
    return (_fd);
}

const std::string& Client::getBuffer() const
{
    return (_buffer);
}

bool Client::isDisconnected() const
{
    return (_disconnected);
}

void Client::addData(const char *data, int bytes)
{
    _buffer.append(data, bytes);
}

void Client::disconnect()
{
    _disconnected = true;
}

bool Client::getNextMessage(std::string& message)
{
    size_t pos = _buffer.find("\n");

    if (pos == std::string::npos)
        return (false);

    message = _buffer.substr(0, pos);
    _buffer.erase(0, pos + 2);

    return (true);
}
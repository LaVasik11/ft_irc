#include "Server.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstdio>
#include <cstring>
#include <ostream>
#include <iostream>


Server::Server(int port, std::string password) : _serverFd(-1), _port(port), _password(password), _running(false) {}

Server::~Server()
{
    for (size_t i = 0; i < _clients.size(); i++)
        close(_clients[i].getFd());

    if (_serverFd != -1)
        close(_serverFd);
}

bool Server::setNonBlocking(int fd)
{
    int	flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1)
        return (false);

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        return (false);

    return (true);
}

int Server::createSocket()
{
    int	fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1)
    {
        perror("socket");
        return (-1);
    }

    int opt = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt");
        close(fd);
        return (-1);
    }

    if (!setNonBlocking(fd))
    {
        perror("fcntl");
        close(fd);
        return (-1);
    }

    struct sockaddr_in	address;
    std::memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_port = htons(_port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        perror("bind");
        close(fd);
        return (-1);
    }

    if (listen(fd, 10) == -1)
    {
        perror("listen");
        close(fd);
        return (-1);
    }

    return (fd);
}

bool Server::init()
{
    _serverFd = createSocket();

    if (_serverFd == -1)
        return (false);

    _running = true;
    return (true);
}

void Server::acceptNewClients()
{
    while (true)
    {
        int clientFd = accept(_serverFd, NULL, NULL);

        if (clientFd == -1)
        {
            if (errno == EINTR)
                continue;

            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;

            perror("accept");
            break;
        }

        if (!setNonBlocking(clientFd))
        {
            perror("fcntl");
            close(clientFd);
            continue;
        }

        _clients.push_back(Client(clientFd));

        std::cout << "New client connected: " << clientFd << std::endl;
    }
}

void Server::processClientMessages(Client& client)
{
    std::string rawMessage;

    while (client.getNextMessage(rawMessage))
    {
        Message message;

        if (!_parser.parse(rawMessage, message))
            continue;

        _commandHandler.handle(*this, client, message);
    }
}

void Server::handleClientData(Client& client)
{
    char	buffer[1024];

    while (true)
    {
        ssize_t	bytes = recv(client.getFd(), buffer, sizeof(buffer), 0);

        if (bytes > 0)
        {
            client.addData(buffer, bytes);
            continue;
        }

        if (bytes == 0)
        {
            client.disconnect();
            break;
        }

        if (errno == EINTR)
            continue;

        if (errno == EAGAIN || errno == EWOULDBLOCK)
            break;

        perror("recv");
        client.disconnect();
        break;
    }
	processClientMessages(client);
}

void Server::handleServerEvent(short events)
{
    if (events & (POLLERR | POLLHUP | POLLNVAL))
    {
        std::cerr << "Server socket error" << std::endl;
        _running = false;
        return;
    }

    if (events & POLLIN)
        acceptNewClients();
}

void Server::handleClientEvent(Client& client, short events)
{
    if (events & (POLLERR | POLLNVAL))
    {
        client.disconnect();
        return;
    }

    if (events & POLLIN)
        handleClientData(client);

    if (events & POLLHUP)
        client.disconnect();
}

void Server::removeDisconnectedClients()
{
    for (size_t i = 0; i < _clients.size(); )
    {
        if (_clients[i].isDisconnected())
        {
            std::cout << "Client disconnected: " << _clients[i].getFd() << std::endl;

            close(_clients[i].getFd());
            _clients.erase(_clients.begin() + i);
        }
        else
            i++;
    }
}

void Server::run()
{
	std::vector<struct pollfd>	pollFds;

    while (_running)
    {
		pollFds.clear();

		struct pollfd				serverPoll;
        serverPoll.fd = _serverFd;
        serverPoll.events = POLLIN;
        serverPoll.revents = 0;

        pollFds.push_back(serverPoll);

        for (size_t i = 0; i < _clients.size(); i++)
        {
            struct pollfd	clientPoll;
            clientPoll.fd = _clients[i].getFd();
            clientPoll.events = POLLIN;
            clientPoll.revents = 0;

            pollFds.push_back(clientPoll);
        }

        int	ret = poll(&pollFds[0], pollFds.size(), -1);

        if (ret == -1)
        {
            if (errno == EINTR)
                continue;

            perror("poll");
            break;
        }

        if (pollFds[0].revents)
            handleServerEvent(pollFds[0].revents);

        for (size_t i = 1; i < pollFds.size(); i++)
        {
            if (pollFds[i].revents)
    			handleClientEvent(_clients[i - 1], pollFds[i].revents);
        }

        removeDisconnectedClients();
    }
}
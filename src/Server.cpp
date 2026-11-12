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
    for (std::list<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        close(it->getFd());

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

    if (client.isDisconnected())
        return;

    if (events & POLLOUT)
        sendClientData(client);

    if (events & POLLHUP)
        client.disconnect();
}

void Server::removeDisconnectedClients()
{
    std::list<Client>::iterator it = _clients.begin();

    while (it != _clients.end())
    {
        if (it->isDisconnected())
        {
            std::cout << "Client disconnected: " << it->getFd() << std::endl;

            std::vector<Channel*> channels = it->getChannels();

            for (size_t i = 0; i < channels.size(); ++i)
			{
                channels[i]->removeMember(*it);
				if (channels[i]->getMembersCount() == 0)
					removeChannel(channels[i]);
			}
            close(it->getFd());
            it = _clients.erase(it);
        }
        else
            ++it;
    }
}

void Server::sendClientData(Client& client)
{
    const std::string& buffer = client.getSendBuffer();

    if (buffer.empty())
        return;

    ssize_t sent = send(client.getFd(), buffer.c_str(), buffer.size(), MSG_NOSIGNAL);

    if (sent > 0)
        client.removeSentData(sent);
    else if (sent == -1 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
        client.disconnect();
}

bool Server::isNickInUse(std::string nick)
{
    for (std::list<Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->getNick() == nick)
            return (true);
    }
    return (false);
}

bool Server::tryClientPassword(std::string password)
{
	if (_password == password)
		return (true);
	return (false);
}

Channel* Server::findChannel(const std::string& title)
{
    for (std::list<Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        if (it->getTitle() == title)
            return (&(*it));
    }
    return (NULL);
}

Channel& Server::addChannel(Client& owner, std::string title)
{
    _channels.push_back(Channel(owner, title));
    Channel& channel = _channels.back();
    channel.addMember(owner);
    channel.addOperator(owner);
    owner.addChannel(channel);

    return (channel);
}

void Server::removeChannel(Channel* channel)
{
    for (std::list<Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        if (&(*it) == channel)
        {
            _channels.erase(it);
            return;
        }
    }
}

Client* Server::findClient(std::string client)
{
	for (std::list<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->getNick() == client)
            return (&(*it));
    }
    return (NULL);
}

void Server::run()
{
	std::vector<struct pollfd>	pollFds;
	std::vector<Client*>		pollClients;

    while (_running)
    {
		pollFds.clear();
		pollClients.clear();

		struct pollfd serverPoll;
        serverPoll.fd = _serverFd;
        serverPoll.events = POLLIN;
        serverPoll.revents = 0;

        pollFds.push_back(serverPoll);

        for (std::list<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		{
			struct pollfd clientPoll;

			clientPoll.fd = it->getFd();
			clientPoll.events = POLLIN;
			clientPoll.revents = 0;

			if (!it->getSendBuffer().empty())
				clientPoll.events |= POLLOUT;

			pollFds.push_back(clientPoll);
			pollClients.push_back(&(*it));
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

        for (size_t i = 1; i < pollFds.size(); ++i)
		{
			if (pollFds[i].revents)
				handleClientEvent(*pollClients[i - 1], pollFds[i].revents);
		}

        removeDisconnectedClients();
    }
}
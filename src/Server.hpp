#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <poll.h>
#include "Client.hpp"
#include "MessageParser.hpp"
#include "CommandHandler.hpp"
#include "Channel.hpp"


class Server
{
public:
    Server(int port, std::string password);
    ~Server();

    bool	init();
    void	run();
	bool	isNickInUse(std::string nick);
	bool	tryClientPassword(std::string password);
	void	addChannel(Client& owner, std::string title);

private:
    int                 	_serverFd;
    int                 	_port;
	std::string				_password;
    bool                	_running;
    std::vector<Client>		_clients;
	std::vector<Channel>	_channels;
	MessageParser  			_parser;
	CommandHandler			_commandHandler;

    int		createSocket();
    bool	setNonBlocking(int fd);

    void	acceptNewClients();
    void	handleClientData(Client& client);
	void	processClientMessages(Client& client);
    void	removeDisconnectedClients();
    void	handleServerEvent(short events);
    void	handleClientEvent(Client& client, short events);
	void	sendClientData(Client& client);
};

#endif
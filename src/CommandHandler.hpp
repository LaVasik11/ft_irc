#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "Message.hpp"
#include "Client.hpp"

class Server;

class CommandHandler
{
public:
    void	handle(Server& server, Client& client, const Message& message);
	void	sendMessage(Client& client, std::string msg);
	void	sendNumeric(Client& client, int code, const std::string& message);

private:
    void	handleNick(Server& server, Client& client, const Message& message);
    void	handleUser(Server& server, Client& client, const Message& message);
    void	handleJoin(Server& server, Client& client, const Message& message);
    void	handlePrivmsg(Server& server, Client& client, const Message& message);
    void	handleQuit(Server& server, Client& client, const Message& message);
    void	handlePing(Server& server, Client& client, const Message& message);
};

#endif
#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "Message.hpp"
#include "Client.hpp"
#include <vector>
#include <string>

class Server;

class CommandHandler
{
public:
    void						handle(Server& server, Client& client, const Message& message);
	void						sendMessage(Client& client, std::string msg);
	void						sendNumeric(Client& client, int code, const std::string& message);

private:
    void						handleNick(Server& server, Client& client, const Message& message);
    void						handleUser(Server& server, Client& client, const Message& message);
	void						handlePass(Server& server, Client& client, const Message& message);
    void						handleJoin(Server& server, Client& client, const Message& message);
    void						handlePrivmsg(Server& server, Client& client, const Message& message);
    void						handleQuit(Server& server, Client& client, const Message& message);
    void						handlePing(Server& server, Client& client, const Message& message);
	void						handleMode(Server& server, Client& client, const Message& message);

	void						tryRegister(Client& client);
	std::vector<std::string>	split(const std::string& str, char delimiter);
	void						joinChannel(Server& server, Client& client, const std::string& title, const std::string& key);
	void						handleJoinZero(Server& server, Client& client);
};

#endif
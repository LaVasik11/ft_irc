#include "CommandHandler.hpp"
#include "Server.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include "IRCReplies.hpp"


void CommandHandler::handle(Server& server, Client& client, const Message& message)
{
    if (message.command == "NICK")
        handleNick(server, client, message);
    else if (message.command == "USER")
        handleUser(server, client, message);
	else if (message.command == "PASS")
		handlePass(server, client, message);
    else if (message.command == "JOIN")
        handleJoin(server, client, message);
    else if (message.command == "PRIVMSG")
        handlePrivmsg(server, client, message);
    else if (message.command == "QUIT")
        handleQuit(server, client, message);
    else if (message.command == "PING")
        handlePing(server, client, message);
	else if (message.command == "MODE")
		handleMode(server, client, message);
	else
	{
		std::ostringstream	error_msg;

		error_msg << message.command << " :Unknown command";
		sendNumeric(client, IRC::ERR_UNKNOWNCOMMAND, error_msg.str());
	}
}

void	CommandHandler::sendMessage(Client& client, std::string msg)
{
	client.queueMessage(msg);
}

void CommandHandler::sendNumeric(Client& client, int code, const std::string& message)
{
    std::ostringstream	response;

    response << ":server " << std::setw(3) << std::setfill('0') \
	<< code << " " << client.getNick() << " " << message << "\r\n";

    sendMessage(client, response.str());
}

static bool isValidNick(const std::string& nick)
{
    if (nick.empty() || nick.size() > 9)
        return (false);

    if (!std::isalpha(nick[0]) && std::string("[]\\`_^{|}").find(nick[0]) == std::string::npos)
        return (false);

    for (size_t i = 1; i < nick.size(); ++i)
    {
        char c = nick[i];

        if (!std::isalnum(c) && std::string("-[]\\`_^{|}").find(c) == std::string::npos)
            return (false);
    }

    return (true);
}

void CommandHandler::tryRegister(Client& client)
{
	if (!client.getUsername().empty() && (!client.getNick().empty() && client.getNick() != "*") && client.hasPassword())
	{
		client.registerClient();
		sendNumeric(client, IRC::RPL_WELCOME, ":Welcome to the IRC Network " + client.getNick());
	}
}

void CommandHandler::handleNick(Server& server, Client& client, const Message& message)
{
    if (message.params.empty())
    {
        sendNumeric(client, IRC::ERR_NONICKNAMEGIVEN, ":No nickname given");
        return;
    }

    if (!isValidNick(message.params[0]))
    {
        sendNumeric(client, IRC::ERR_ERRONEUSNICKNAME, message.params[0] + " :Erroneous nickname");
        return;
    }

    if (server.isNickInUse(message.params[0]))
    {
        sendNumeric(client, IRC::ERR_NICKNAMEINUSE, message.params[0] + " :Nickname is already in use");
        return;
    }

    client.setNick(message.params[0]);
	tryRegister(client);
}

void CommandHandler::handleUser(Server& server, Client& client, const Message& message)
{
	if (client.isRegistered() || !client.getUsername().empty())
    {
        sendNumeric(client, IRC::ERR_ALREADYREGISTRED, ":You may not reregister");
        return;
    }

    if (message.params.size() != 4)
    {
        sendNumeric(client, IRC::ERR_NEEDMOREPARAMS, "USER :Not enough parameters");
        return;
    }

    const std::string& username = message.params[0];
    const std::string& mode = message.params[1];
    const std::string& unused = message.params[2];
    const std::string& realname = message.params[3];
	(void)unused;
	(void)mode;
	(void)server;

    client.setUsername(username);
    client.setRealname(realname);
    tryRegister(client);
}


void CommandHandler::handlePass(Server& server, Client& client, const Message& message)
{
	if (client.isRegistered() || client.hasPassword())
	{
		sendNumeric(client, IRC::ERR_ALREADYREGISTRED, "PASS :You may not reregister");
		return;
	}

	if (message.params.empty())
	{
		sendNumeric(client, IRC::ERR_NEEDMOREPARAMS, "PASS :Not enough parameters");
		return;
	}

	if (!server.tryClientPassword(message.params[0]))
	{
		sendNumeric(client, IRC::ERR_PASSWDMISMATCH, "PASS :Password incorrect");
		return;
	}

	client.setPassword();
	tryRegister(client);
}

void CommandHandler::handleJoin(Server& server, Client& client, const Message& message)
{
	if (!client.isRegistered())
	{
		sendNumeric(client, IRC::ERR_NOTREGISTERED, "JOIN :You have not registered");
		return;
	}

	(void)server;
	(void)client;
	(void)message;
}


void CommandHandler::handlePrivmsg(Server& server, Client& client, const Message& message)
{
	if (!client.isRegistered())
	{
		sendNumeric(client, IRC::ERR_NOTREGISTERED, "PRIVMSG :You have not registered");
		return;
	}
	(void)server;
	(void)client;
	(void)message;
}

void CommandHandler::handleQuit(Server& server, Client& client, const Message& message)
{
	(void)server;
	(void)client;
	(void)message;
}

void CommandHandler::handlePing(Server& server, Client& client, const Message& message)
{
	(void)server;
	(void)client;
	(void)message;
}

void CommandHandler::handleMode(Server& server, Client& client, const Message& message)
{
	if (!client.isRegistered())
	{
		sendNumeric(client, IRC::ERR_NOTREGISTERED, "MODE :You have not registered");
		return;
	}
	(void)server;
	(void)client;
	(void)message;
}

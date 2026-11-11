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

void CommandHandler::joinChannel(Server& server, Client& client, const std::string& title, const std::string& key)
{
    if (title.empty() || title.size() > 50 || (title[0] != '#' && title[0] != '&'))
    {
        sendNumeric(client, IRC::ERR_BADCHANMASK, title + " :Bad Channel Mask");
        return;
    }

    for (size_t i = 0; i < title.size(); ++i)
    {
        if (title[i] == ' ' || title[i] == ',' || title[i] == '\r' \
			|| title[i] == '\n' || title[i] == '\0' || title[i] == '\a')
        {
            sendNumeric(client, IRC::ERR_BADCHANMASK, title + " :Bad Channel Mask");
            return;
        }
    }

    Channel* channel = server.findChannel(title);

    if (!channel)
        channel = &server.addChannel(client, title);
    else
    {
        if (channel->hasMember(client))
            return;

        if (channel->hasPassword() && channel->getPassword() != key)
            return (sendNumeric(client, IRC::ERR_BADCHANNELKEY, title + " :Cannot join channel (+k)"));


        if (channel->isInviteOnly())
            return (sendNumeric(client, IRC::ERR_INVITEONLYCHAN, title + " :Cannot join channel (+i)"));


        if (channel->hasUserLimit() && channel->getMembersCount() >= channel->getUserLimit())
            return (sendNumeric(client, IRC::ERR_CHANNELISFULL, title + " :Cannot join channel (+l)"));

        channel->addMember(client);
        client.addChannel(*channel);
    }

    std::string	prefix = ":" + client.getNick() + "!" + client.getUsername() + "@localhost";
    std::string	joinMessage = prefix + " JOIN :" + title + "\r\n";
    const std::vector<Client*>&	members = channel->getMembers();

    for (size_t i = 0; i < members.size(); ++i)
        members[i]->queueMessage(joinMessage);

    if (!channel->getTopic().empty())
        sendNumeric(client, IRC::RPL_LIST, title + " :" + channel->getTopic());
    else
        sendNumeric(client, IRC::RPL_NOTOPIC, title + " :No topic is set");

    std::string	names;

    for (size_t i = 0; i < members.size(); ++i)
    {
        if (!names.empty())
            names += " ";

        if (channel->hasOperator(*members[i]))
            names += "@";

        names += members[i]->getNick();
    }

    sendNumeric(client, IRC::RPL_NAMREPLY, "= " + title + " :" + names);
    sendNumeric(client, IRC::RPL_ENDOFNAMES, title + " :End of /NAMES list");
}

void CommandHandler::handleJoinZero(Server& server, Client& client)
{
    std::vector<Channel*> channels = client.getChannels();

    for (size_t i = 0; i < channels.size(); ++i)
    {
        Channel* channel = channels[i];

        std::string message = ":" + client.getNick() + "!" + client.getUsername() \
		+ "@localhost" + " PART " + channel->getTitle() + " :Leaving all channels\r\n";

        const std::vector<Client*>& members = channel->getMembers();

        for (size_t j = 0; j < members.size(); ++j)
            members[j]->queueMessage(message);

        channel->removeMember(client);
        client.removeChannel(*channel);

        if (channel->getMembersCount() == 0)
            server.removeChannel(channel);
    }
}

std::vector<std::string> CommandHandler::split(const std::string& str, char delimiter)
{
    std::vector<std::string> result;
    size_t start = 0;
    size_t end;

    while ((end = str.find(delimiter, start)) != std::string::npos)
    {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
    }

    result.push_back(str.substr(start));
    return (result);
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
        return (sendNumeric(client, IRC::ERR_NONICKNAMEGIVEN, ":No nickname given"));

    if (!isValidNick(message.params[0]))
        return (sendNumeric(client, IRC::ERR_ERRONEUSNICKNAME, message.params[0] + " :Erroneous nickname"));

    if (server.isNickInUse(message.params[0]))
        return (sendNumeric(client, IRC::ERR_NICKNAMEINUSE, message.params[0] + " :Nickname is already in use"));

    client.setNick(message.params[0]);
	tryRegister(client);
}

void CommandHandler::handleUser(Server& server, Client& client, const Message& message)
{
	if (client.isRegistered() || !client.getUsername().empty())
        return (sendNumeric(client, IRC::ERR_ALREADYREGISTRED, ":You may not reregister"));


    if (message.params.size() != 4)
    	return (sendNumeric(client, IRC::ERR_NEEDMOREPARAMS, "USER :Not enough parameters"));


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
		return (sendNumeric(client, IRC::ERR_ALREADYREGISTRED, "PASS :You may not reregister"));

	if (message.params.empty())
		return (sendNumeric(client, IRC::ERR_NEEDMOREPARAMS, "PASS :Not enough parameters"));


	if (!server.tryClientPassword(message.params[0]))
		return (sendNumeric(client, IRC::ERR_PASSWDMISMATCH, "PASS :Password incorrect"));

	client.setPassword();
	tryRegister(client);
}

void CommandHandler::handleJoin(Server& server, Client& client, const Message& message)
{
	if (!client.isRegistered())
		return (sendNumeric(client, IRC::ERR_NOTREGISTERED, "JOIN :You have not registered"));

	if (message.params.empty())
        return (sendNumeric(client, IRC::ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters"));

    if (message.params[0] == "0")
        return handleJoinZero(server, client);

    std::vector<std::string> channels = split(message.params[0], ',');
    std::vector<std::string> keys;

    if (message.params.size() > 1)
        keys = split(message.params[1], ',');

    for (size_t i = 0; i < channels.size(); ++i)
    {
        std::string key;

        if (i < keys.size())
            key = keys[i];

        joinChannel(server, client, channels[i], key);
    }
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

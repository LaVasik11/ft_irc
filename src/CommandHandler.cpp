#include "CommandHandler.hpp"
#include "Server.hpp"

#include <iostream>

void CommandHandler::handle(Server& server, Client& client, const Message& message)
{
	(void)server;

    if (message.command == "PING")
    {
        std::cout << "PING from " << client.getFd() << std::endl;
    }
    else if (message.command == "NICK")
    {
        std::cout << "NICK from " << client.getFd() << std::endl;
    }
    else if (message.command == "USER")
    {
        std::cout << "USER from " << client.getFd() << std::endl;
    }
    else if (message.command == "JOIN")
    {
        std::cout << "JOIN from " << client.getFd() << std::endl;
    }
    else if (message.command == "PRIVMSG")
    {
        std::cout << "PRIVMSG from " << client.getFd() << std::endl;
    }
    else
    {
        std::cout << "Unknown command: " << message.command << std::endl;
    }
}
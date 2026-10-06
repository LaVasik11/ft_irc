#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <vector>
#include <iostream>
#include <errno.h>
#include <cstdio>
#include <algorithm>
#include <fcntl.h>
#include "Client.hpp"


bool send_all(int fd, const char *buffer, int bytes)
{
    int sent = 0;

    while (sent < bytes)
    {
        int n = send(fd, buffer + sent, bytes - sent, 0);

        if (n < 0)
		{
			if (errno == EINTR)
				continue;
			return false;
		}
		if (n == 0)
			return false;

        sent += n;
    }

    return true;
}

void	add_to_remove(std::vector<int>& to_remove, int fd)
{
	if (std::find(to_remove.begin(), to_remove.end(), fd) == to_remove.end())
		to_remove.push_back(fd);
}

int	create_server_socet()
{
	int server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (server_fd == -1)
	{
		perror("socket: ");
		return (-1);
	}
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(6667);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		perror("setsockopt: ");
    	close(server_fd);
    	return (-1);
	}

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1)
	{
		perror("bind: ");
		close(server_fd);
		return (-1);
	}
    if (listen(server_fd, 10) == -1)
	{
		perror("listen: ");
		close(server_fd);
		return (-1);
	}
	return (server_fd);
}

bool set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1)
        return false;

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        return false;

    return true;
}

int main()
{
	bool	running = true;
	int		server_fd = create_server_socet();

	if (server_fd == -1)
		return (1);

	std::vector<int>			to_remove;
	std::vector<struct pollfd>	fds;
	std::vector<Client>			clients;
	
	fds.push_back((struct pollfd){server_fd, POLLIN, 0});

	while (running)
	{
		int ret = poll(fds.data(), fds.size(), -1);
		
		if (ret == -1)
		{
			if (errno == EINTR)
        		continue;
    		perror("poll: ");
			break;
		}
		
		size_t	size = fds.size();
		for (size_t i = 0; i < size; i++)
		{
			if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
			{
				if (fds[i].fd == server_fd)
					running = false;
				else
					add_to_remove(to_remove, fds[i].fd);
				continue;
			}

			if (!(fds[i].revents & POLLIN))
				continue;

			if (fds[i].fd == server_fd)
			{
				while (true)
				{
					int		client_fd = accept(server_fd, 0, 0);
					if (client_fd == -1)
					{
						if (errno == EAGAIN || errno == EWOULDBLOCK)
							break;

						perror("accept: ");
						continue;
					}

					if (!set_nonblocking(client_fd))
					{
						perror("fcntl: ");
						close(client_fd);
						continue;
					}
					fds.push_back((struct pollfd){client_fd, POLLIN, 0});
					clients.push_back(Client(client_fd));
				}
			}
			else
			{
				char	buffer[1024];
				int		bytes = recv(fds[i].fd, buffer, sizeof(buffer), 0);

				if (bytes < 0)
				{
					if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
						continue;

					perror("recv: ");
					add_to_remove(to_remove, fds[i].fd);
				}
				else if (bytes == 0)
					add_to_remove(to_remove, fds[i].fd);
				else
				{				
					for (size_t j = 0; j < clients.size(); j++)
					{
						if (clients[j].getFd() == fds[i].fd)
						{
							clients[j].addData(buffer, bytes);
							break;
						}
					}
				}
			}
		}

		for (size_t i = 0; i < to_remove.size(); i++)
		{
			int fd = to_remove[i];

			for (size_t j = 0; j < fds.size(); j++)
			{
				if (fds[j].fd == fd)
				{
					close(fd);
					fds.erase(fds.begin() + j);
					break;
				}
			}

			for (size_t j = 0; j < clients.size(); j++)
			{
				if (clients[j].getFd() == fd)
				{
					clients.erase(clients.begin() + j);
					break;
				}
			}
		}
		to_remove.clear();
	}

	close(server_fd);
	return (0);
}

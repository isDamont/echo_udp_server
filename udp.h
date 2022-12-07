#pragma once

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>


#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>



class resolver{
public:
    void set_new_address(std::string host_name_) {
        host_name = host_name_;
   }
    std::string get_last_client_address() {
        if (host_name.empty()){
            return "host name is empty";
        }
        return host_name;
    }
    int get_name_with_gethostbyname()
    {

        socket_wrapper::SocketWrapper sock_wrap;
        const hostent* remote_host{ gethostbyname(host_name.c_str()) };

        if (nullptr == remote_host)
        {
            if (sock_wrap.get_last_error_code())
            {
                std::cerr << sock_wrap.get_last_error_string() << std::endl;
            }

            return EXIT_FAILURE;
        }

        std::cout << "Official name: " << remote_host->h_name << "\n";

        for (const char* const* p_alias = const_cast<const char* const*>(remote_host->h_aliases); *p_alias; ++p_alias)
        {
            std::cout << "# Alternate name: \"" << *p_alias << "\"\n";
        }

        std::cout << std::endl;

        return EXIT_SUCCESS;
    }

    int get_name_with_getaddrinfo()
    {
        // Need for Windows initialization.
        socket_wrapper::SocketWrapper sock_wrap;

        addrinfo hints =
        {
            .ai_flags = AI_CANONNAME,
        };

        // Results.
        addrinfo* servinfo = nullptr;
        int status = 0;

        if ((status = getaddrinfo(host_name.c_str(), nullptr, &hints, &servinfo)) != 0)
        {
            std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
            return EXIT_FAILURE;
        }

        for (auto const* s = servinfo; s != nullptr; s = s->ai_next)
        {
            std::cout << "Canonical name: ";
            if (s->ai_canonname)
                std::cout << s->ai_canonname;
        }
        std::cout << std::endl;

        freeaddrinfo(servinfo);

        return EXIT_SUCCESS;
    }
private:
    std::string host_name;
};

class udp_server {
public:

    udp_server(const int port_) : port(port_), sock({ AF_INET, SOCK_DGRAM, IPPROTO_UDP }) {
       
        std::cout << "Starting server on the port " << port << "...\n";

        if (!sock)
        {
            std::cerr << sock_wrap.get_last_error_string() << std::endl;
             exit(EXIT_FAILURE);
        }

        addr =
        {
            .sin_family = PF_INET,
            .sin_port = htons(port),
        };

        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sock, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) != 0)
        {
            std::cerr << sock_wrap.get_last_error_string() << std::endl;
            // Socket will be closed in the Socket destructor.
            exit(EXIT_FAILURE);
        }

	}
	void wait_for_clients(resolver* res) {
        
        // socket address used to store client address
        struct sockaddr_in client_address = { 0 };
        socklen_t client_address_len = sizeof(sockaddr_in);
        ssize_t recv_len = 0;

        std::cout << "Running echo server...\n" << std::endl;
        char client_address_buf[INET_ADDRSTRLEN];

        while (true)
        {
            // Read content into buffer from an incoming client.
            recv_len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                reinterpret_cast<sockaddr*>(&client_address),
                &client_address_len);

            if (recv_len > 0)
            {
                buffer[recv_len] = '\0';
                std::cout
                    << "Client "
                    << inet_ntop(AF_INET, &client_address.sin_addr, client_address_buf, sizeof(client_address_buf) / sizeof(client_address_buf[0]))
                    << ":" << ntohs(client_address.sin_port)
                    << " is connected"
                    << std::endl;
                ///
                res->set_new_address(client_address_buf);
                res->get_name_with_getaddrinfo();
                res->get_name_with_gethostbyname();
                ///
                std::cout
                    << "His datagram is"
                    << "\n'''\n"
                    << buffer
                    << "\n'''\n"
                    << "[length = "
                    << recv_len
                    << "]"
                    << std::endl;
            if (!memcmp(buffer, "exit", 4)) {
                    exit(EXIT_SUCCESS);
                }

                sendto(sock, buffer, recv_len, 0, reinterpret_cast<const sockaddr*>(&client_address),
                    client_address_len);
            }

            std::cout << std::endl;
        }

	}
private:
    char buffer[256];
	const int port;
	socket_wrapper::SocketWrapper sock_wrap;
    socket_wrapper::Socket sock;
    sockaddr_in addr;
};


class host {
public:
    host() : udp_s (nullptr), res(nullptr) {}
    ~host() {
        if(udp_s != nullptr){ delete udp_s; }
        if (res != nullptr) { delete res; }
    }
    void start(const int port_) {
        if(udp_s == nullptr){
        udp_s = new udp_server(port_);
        }
    }

    void wait_for_clients() {
        if (udp_s != nullptr) {
            res = new resolver;
            udp_s->wait_for_clients(res);
        }
    }

private:
    udp_server* udp_s;
    resolver* res;
};
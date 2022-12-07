#include "udp.h"


int main(int argc, char const *argv[])
{

    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    const int port { std::stoi(argv[1])};
    auto* test = new host;
    test->start(port);
    test->wait_for_clients();
    delete test;

    return EXIT_SUCCESS;
}


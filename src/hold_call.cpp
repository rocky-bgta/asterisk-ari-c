
#include "../include/client.h"

boost::asio::io_context ios;
int main() {
    // Static values for ARI connection
    const std::string host = "192.168.0.180";
    const std::string port = "8088";
    const std::string username = "asterisk";
    const std::string password = "secret";
    const std::string stasisapp = "my-stasis-app";

    aricpp::Client client(ios, host, port, username, password, stasisapp);
    client.Connect([](boost::system::error_code e) {
        if (e) std::cerr << "Error connecting: " << e.message() << '\n';
        else std::cout << "Connected" << '\n';
        });

    ios.run();
    return 0;
}

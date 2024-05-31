
#include "../include/client.h"
#include <aricpp/arimodel.h>
#include <nlohmann/json.hpp>

boost::asio::io_context ios;
using namespace aricpp;
using json = nlohmann::json;

void Dump(const json& j) {
    std::cout << j.dump(4) << '\n';  // Pretty print JSON with an indentation of 4 spaces
}

int main() {
    // Static values for ARI connection
    //const std::string host = "192.168.0.180";
    const std::string host = "192.168.30.87";
    const std::string port = "18088";
    const std::string username = "asterisk";
    const std::string password = "secret";
    const std::string stasisapp = "my-stasis-app";

    aricpp::Client client(ios, host, port, username, password, stasisapp);
    
    client.Connect([&](boost::system::error_code e) {
        if (e)
        {
            std::cerr << "Asterisk Error connecting: " << e.message() << '\n';
            return;
        }
        std::cout << "Asterisk Connected" << '\n';

        client.OnEvent("StasisStart", [](const JsonTree& e) {
            Dump(e); // print the json on the console
            auto channelId = Get<std::string>(e, { "channel", "id" });
            auto callerExtension = Get<std::string>(e, { "channel", "caller","number" });
            auto targetExtension = Get<std::string>(e, { "channel", "dialplan","exten"});
            std::cout << "Target Extension number " << targetExtension << " number\n";
            std::cout << "Channel id " << channelId << " entered stasis application\n";
            });

        client.OnEvent("StasisEnd", [](const JsonTree& e) {
            Dump(e); // print the json on the console
            auto id = Get<std::string>(e, { "channel", "id" });
            std::cout << "Channel id " << id << " end stasis application\n";
            });

        });

        //AriModel channels(client);

   

    ios.run();
    return 0;
}

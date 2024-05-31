#include <iostream>
#include <string>
#include <httplib.h>
#include <json/json.h>

// Helper function to make a POST request to the Asterisk server
Json::Value postRequest(const std::string& url, const std::string& endpoint, const std::string& body, const std::string& user, const std::string& password) {
    httplib::Client cli(url.c_str());
    cli.set_basic_auth(user.c_str(), password.c_str());

    auto res = cli.Post(endpoint.c_str(), body, "application/json");
    if (!res || res->status != 200) {
        std::cerr << "Error: " << res->status << " - " << res->body << std::endl;
        exit(1);
    }

    Json::Value jsonData;
    std::istringstream(res->body) >> jsonData;
    return jsonData;
}

int main() {
    std::string server_url = "http://localhost:8088"; // ARI server URL
    std::string user = "asterisk";
    std::string password = "your_ari_password";

    // Step 1: Create a channel
    std::string create_channel_endpoint = "/ari/channels";
    Json::Value channel_data;
    channel_data["endpoint"] = "SIP/1000";
    channel_data["extension"] = "1000";
    channel_data["context"] = "default";
    channel_data["priority"] = 1;
    channel_data["app"] = "your_app_name";

    Json::FastWriter fastWriter;
    std::string channel_body = fastWriter.write(channel_data);

    Json::Value channel_response = postRequest(server_url, create_channel_endpoint, channel_body, user, password);
    std::string channel_id = channel_response["id"].asString();
    std::cout << "Channel created with ID: " << channel_id << std::endl;

    // Step 2: Create a bridge
    std::string create_bridge_endpoint = "/ari/bridges";
    Json::Value bridge_data;
    bridge_data["type"] = "mixing";

    std::string bridge_body = fastWriter.write(bridge_data);

    Json::Value bridge_response = postRequest(server_url, create_bridge_endpoint, bridge_body, user, password);
    std::string bridge_id = bridge_response["id"].asString();
    std::cout << "Bridge created with ID: " << bridge_id << std::endl;

    // Step 3: Add the channel to the bridge
    std::string add_channel_to_bridge_endpoint = "/ari/bridges/" + bridge_id + "/addChannel";
    Json::Value add_channel_data;
    add_channel_data["channel"] = channel_id;

    std::string add_channel_body = fastWriter.write(add_channel_data);

    postRequest(server_url, add_channel_to_bridge_endpoint, add_channel_body, user, password);
    std::cout << "Channel " << channel_id << " added to bridge " << bridge_id << std::endl;

    return 0;
}

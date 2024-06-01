#include <iostream>
#include <string>
#include <httplib.h>
#include <json/json.h>

// Function to register the ARI application
void registerARIApplication(const std::string& server_url, const std::string& user, const std::string& password) {
    // Placeholder function for ARI application registration
    // In reality, ARI application registration is handled in the Asterisk config files (ari.conf and extensions.conf)
    // This function can be used to validate the connection and credentials
    httplib::Client cli(server_url.c_str());
    cli.set_basic_auth(user.c_str(), password.c_str());

    auto res = cli.Get("/ari/asterisk/info");
    if (res && res->status == 200) {
        std::cout << "ARI Application registered successfully." << std::endl;
    } else {
        std::cerr << "Failed to register ARI Application. Check server URL, username, or password." << std::endl;
        exit(1);
    }
}

// Helper function to make a POST request to the Asterisk server
Json::Value postRequest(const std::string& url, const std::string& endpoint, const std::string& body, const std::string& user, const std::string& password) {
    httplib::Client cli(url.c_str());
    cli.set_basic_auth(user.c_str(), password.c_str());

    auto res = cli.Post(endpoint.c_str(), body, "application/json");
    if (!res || res->status != 200) {
        std::cerr << "Error: " << (res ? res->status : 0) << " - " << (res ? res->body : "No response") << std::endl;
        exit(1);
    }

    Json::Value jsonData;
    std::istringstream(res->body) >> jsonData;
    return jsonData;
}

// Function to handle ARI events
void handleARIEvents(const std::string& server_url, const std::string& user, const std::string& password) {
    httplib::Server svr;

    svr.Post("/ari/events", [&](const httplib::Request& req, httplib::Response& res) {
        // Parse the incoming event
        Json::Value event;
        std::istringstream(req.body) >> event;

        // Check if the event is a StasisStart event
        if (event["type"].asString() == "StasisStart") {
            std::string channel_id = event["channel"]["id"].asString();
            std::string target_extension = event["channel"]["dialplan"]["exten"].asString();

            std::cout << "Incoming call on channel " << channel_id << " to extension " << target_extension << std::endl;

            // Step 1: Create a new channel
            std::string create_channel_endpoint = "/ari/channels";
            Json::Value channel_data;
            channel_data["endpoint"] = "SIP/" + target_extension;
            channel_data["extension"] = target_extension;
            channel_data["context"] = "default";
            channel_data["priority"] = 1;
            channel_data["app"] = "my-stasis-app";

            Json::FastWriter fastWriter;
            std::string channel_body = fastWriter.write(channel_data);

            Json::Value channel_response = postRequest(server_url, create_channel_endpoint, channel_body, user, password);
            std::string new_channel_id = channel_response["id"].asString();
            std::cout << "Channel created with ID: " << new_channel_id << std::endl;

            // Step 2: Create a bridge
            std::string create_bridge_endpoint = "/ari/bridges";
            Json::Value bridge_data;
            bridge_data["type"] = "mixing";

            std::string bridge_body = fastWriter.write(bridge_data);

            Json::Value bridge_response = postRequest(server_url, create_bridge_endpoint, bridge_body, user, password);
            std::string bridge_id = bridge_response["id"].asString();
            std::cout << "Bridge created with ID: " << bridge_id << std::endl;

            // Step 3: Add the original channel to the bridge
            std::string add_channel_to_bridge_endpoint = "/ari/bridges/" + bridge_id + "/addChannel";
            Json::Value add_channel_data;
            add_channel_data["channel"] = channel_id;

            std::string add_channel_body = fastWriter.write(add_channel_data);

            postRequest(server_url, add_channel_to_bridge_endpoint, add_channel_body, user, password);
            std::cout << "Channel " << channel_id << " added to bridge " << bridge_id << std::endl;

            // Step 4: Add the new channel to the bridge
            Json::Value add_new_channel_data;
            add_new_channel_data["channel"] = new_channel_id;

            std::string add_new_channel_body = fastWriter.write(add_new_channel_data);

            postRequest(server_url, add_channel_to_bridge_endpoint, add_new_channel_body, user, password);
            std::cout << "New channel " << new_channel_id << " added to bridge " << bridge_id << std::endl;
        }

        res.set_content("OK", "text/plain");
    });

    std::cout << "Server listening on port 12345" << std::endl;
    svr.listen("0.0.0.0", 8088);
}

int main() {
    std::string server_url = "http://192.168.0.132:8088"; // ARI server URL
    std::string user = "asterisk";
    std::string password = "secret";

    // Register the ARI application
    registerARIApplication(server_url, user, password);

    // Handle ARI events
    handleARIEvents(server_url, user, password);

    return 0;
}

#include <jsoncpp/json/json.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <iomanip>
#include <sstream>
#include <iterator>
#include <map>

std::map<std::string, std::string> parseQueryString(const std::string& query) {
    std::map<std::string, std::string> data;
    std::string key, value;
    size_t pos = 0;

    while (pos < query.size()) {
        size_t ampersandPos = query.find('&', pos);
        if (ampersandPos == std::string::npos) {
            ampersandPos = query.size();
        }
        size_t equalsPos = query.find('=', pos);
        if (equalsPos != std::string::npos && equalsPos < ampersandPos) {
            key = query.substr(pos, equalsPos - pos);
            value = query.substr(equalsPos + 1, ampersandPos - equalsPos - 1);
            data[key] = value;
        }
        pos = ampersandPos + 1;
    }

    return data;
}

std::string getCurrentTimeStamp() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void writeToLog(const std::string &message) {
    std::ofstream logFile("/fibudata/app_log.txt", std::ios_base::app);  // appending to the log file
    if (logFile.is_open()) {
        logFile << getCurrentTimeStamp() << " - " << message << std::endl;
        logFile.close();
    }
}

 //     "Belegdatum" : "1999-12-13",
 //     "Belegnummer" : "67",
 //     "Betrag" : "567",
 //     "HabenKonto" : "5657",
 //     "Mandant" : "678",
 //     "SollKonto" : "56567",
 //     "Text" : "gughj",
 //     "TimeStamp" : "2023-10-28 12:33:18",
 //     "User" : "exampleUser"


std::vector<Json::Value> collectJournalData(const Json::Value& data, const std::string& client) {
    std::vector<Json::Value> result;

    if (!data.isArray()) {
        std::cerr << "Expected a JSON array, but received a different type." << std::endl;
        writeToLog("Expected a JSON array, but received a different type.");

        return result; // Return an empty result
    }
    for (const auto& entry : data) {
        if (entry["Mandant"].asString() == client) {
            Json::Value newEntry;
            newEntry["Belegdatum"] = entry["Belegdatum"];
            newEntry["Belegnummer"] = entry["Belegnummer"];
            newEntry["Mandant"] = entry["Mandant"];
            newEntry["Text"] = entry["Text"];
            newEntry["TimeStamp"] = entry["TimeStamp"];
            newEntry["User"] = entry["User"];
            newEntry["SollKonto"] = entry["SollKonto"];
            newEntry["HabenKonto"] = entry["HabenKonto"];
            newEntry["Betrag"] = entry["Betrag"];
            result.push_back(newEntry);
        }
    }

    return result;
}

int main() {
    char* queryString = getenv("QUERY_STRING");
    if (!queryString) {
        std::cout << "Content-Type: text/plain\r\n\r\n";
        std::cout << "Error: QUERY_STRING not found." << std::endl;
        return 1;
    }

    auto params = parseQueryString(queryString);
    auto clientIt = params.find("client");

    if (clientIt == params.end()) {
        std::cout << "Content-Type: text/plain\r\n\r\n";
        std::cout << "Error: Missing client parameter." << std::endl;
        return 1;
    }

    std::string client = clientIt->second;

    // Read the JSON data from file
    std::ifstream inputFile("/fibudata/"+client+"-bookings.json");
    if (!inputFile.is_open()) {
        std::cout << "{\"status\": \"error\", \"message\": \"Failed to open the bookings file!\"}" << std::endl;
        writeToLog("Failed to open the bookings file!");
        return 0;
    }

    Json::Value data;
    Json::CharReaderBuilder rbuilder;
    std::string errs;

    writeToLog("account: before Json::parseFromStream");

    if (!Json::parseFromStream(rbuilder, inputFile, &data, &errs)) {
        std::cout << "{\"status\": \"error\", \"message\": \"Failed to parse the JSON data\"}" << std::endl;
        writeToLog("Failed to parse the JSON data!");
        return 1;
    }

    inputFile.close();

    auto filteredEntries = collectJournalData(data, client);

    std::cout << "Content-Type: application/json\r\n\r\n";
    // Start the JSON array
    std::cout << "[";
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "    "; // 4 spaces for indentation
    // Output the filtered entries
    writeToLog("account: before filteredEntries output loop");
    for (size_t i = 0; i < filteredEntries.size(); ++i) {
        std::cout << Json::writeString(wbuilder, filteredEntries[i]);
        if (i < filteredEntries.size() - 1) {
            std::cout << ",";
        }
    }
    // End the JSON array
    std::cout << "]";

    return 0;
}


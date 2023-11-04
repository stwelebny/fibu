
/* Copyright (c) 2023 Stefan Welebny
**
** This file is part of [fibu], licensed under the MIT License.
** For full terms see the included LICENSE file.
**
**/

#include <jsoncpp/json/json.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <iomanip>
#include <sstream>
#include <iterator>
#include <map>
#include "hash.h"

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


void verifyBookingChain(const Json::Value& data) {
    if (!data.isArray()) {
        throw std::runtime_error("Expected an array of entries for verification.");
    }

    for (int i = 1; i < data.size(); i++) {
        const Json::Value& entry = data[i];
        const Json::Value& lastEntry = data[i-1];
        const std::string lastEntryData = Json::writeString(Json::StreamWriterBuilder(), lastEntry);
        writeToLog("lastEntryData:" + lastEntryData);
        const std::string hash = calculateSHA256(lastEntryData);
        if (entry["previousHash"] != hash) {
            writeToLog ("Verification failed for entry " + entry["Belegnummer"].asString());
            writeToLog ("Expected hash: " + hash + "\n");
            writeToLog ("Actual 'previousHash' in entry: " + entry["previousHash"].asString() +"\n");

           throw std::runtime_error("Hash verification failed for entry: " + entry["Belegnummer"].asString());
        }
    }
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
    try {
        // Attempt to verify the booking chain
        verifyBookingChain(data);

        // If verification succeeds, send a success message
        std::cout << "Content-Type: application/json\r\n\r\n";
        std::cout << "{\"status\": \"success\", \"message\": \"Das Journal ist konsistent.\"}" << std::endl;
    } catch (const std::exception& e) {
        // If verification fails, send an error message
        std::cout << "Status: 400 Bad Request\r\n";
        std::cout << "Content-Type: application/json\r\n\r\n";
        std::cout << "{\"status\": \"error\", \"message\": \"" << e.what() << "\"}" << std::endl;
    }

    return 0;
}


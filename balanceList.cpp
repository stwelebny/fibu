
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

struct AccountBalance {
    double sollSaldo = 0;
    double habenSaldo = 0;
};

std::map<std::string, AccountBalance> computeBalanceForAccounts(const Json::Value& data, const std::string& client) {
    std::map<std::string, AccountBalance> balances;

    if (!data.isArray()) {
        std::cerr << "Expected a JSON array, but received a different type." << std::endl;
        writeToLog("Expected a JSON array, but received a different type.");

        std::map<std::string, AccountBalance> result;
        return result; // Return an empty result
    }

    for (const auto& entry : data) {
        if (entry["Mandant"].asString() != client) {
            continue;  // Skip entries that don't match the specified client
        }

        std::string accountNumberSoll = entry["SollKonto"].asString();
        std::string accountNumberHaben = entry["HabenKonto"].asString();
        double betrag = std::stod(entry["Betrag"].asString());

        if (!accountNumberSoll.empty()) {
            balances[accountNumberSoll].sollSaldo += betrag;
        }
        
        if (!accountNumberHaben.empty()) {
            balances[accountNumberHaben].habenSaldo += betrag;
        }
    }

    // Compute the net difference for each account and assign to the appropriate column
    for (auto& [account, balance] : balances) {
        if (balance.sollSaldo > balance.habenSaldo) {
            balance.sollSaldo -= balance.habenSaldo;
            balance.habenSaldo = 0;
        } else {
            balance.habenSaldo -= balance.sollSaldo;
            balance.sollSaldo = 0;
        }
    }

    return balances; // This will return sorted by account number for the specified client
}

// Convert the balances map to a JSON array
Json::Value balancesToJsonArray(const std::map<std::string, AccountBalance>& balances) {
    Json::Value jsonArray(Json::arrayValue);

    for (const auto& [account, balance] : balances) {
        Json::Value jsonObject;
        jsonObject["Konto"] = account;
        jsonObject["Soll-Saldo"] = balance.sollSaldo;
        jsonObject["Haben-Saldo"] = balance.habenSaldo;
        jsonArray.append(jsonObject);
    }

    return jsonArray;
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

    writeToLog("account: before computeBalancceForcForAccounts");
    auto balances = computeBalanceForAccounts(data, client);

    std::cout << "Content-Type: application/json\r\n\r\n";
    Json::Value jsonArray = balancesToJsonArray(balances);

    // Serialize and output the JSON array
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "    "; // Optional: 4 spaces for indentation
    std::cout << Json::writeString(wbuilder, jsonArray) << std::endl;

    return 0;
}


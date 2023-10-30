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

struct StructureItem {
    std::string name;
    std::string type;
};

std::string extractFirst3Digits(const std::string& s) {
    for (int i = 0; i < s.size(); i++) {
        if (isdigit(s[i])) {
            return s.substr(i, 3);
        }
    }
    return "";
}

Json::Value iterateThroughAccounts(const std::vector<StructureItem>& structure, const std::map<std::string, AccountBalance>& accountsWithBalances) {
    auto accountIt = accountsWithBalances.begin();

    Json::Value root(Json::arrayValue);

    Json::Value* currentKontenklasse = nullptr;
    Json::Value* current2Digits = nullptr;

    writeToLog("balanceReport, iterateThroughAccounts, entered");
    writeToLog("Size of structure vector: " + std::to_string(structure.size()));
    for (int idx = 0; idx < structure.size(); idx++) {
        const StructureItem& item = structure[idx];

        if (item.type == "Kontenklasse") {
            writeToLog("Handling a Kontenklasse");
            Json::Value kontenklasse(Json::objectValue);
            kontenklasse["name"] = item.name;
            kontenklasse["subClasses"] = Json::Value(Json::arrayValue);

            root.append(kontenklasse);
            currentKontenklasse = &root[root.size() - 1];
        } else if (item.type == "2Digits") {
            writeToLog("Handling a 2Digits");
            Json::Value twoDigits(Json::objectValue);
            twoDigits["name"] = item.name;
            twoDigits["subClasses"] = Json::Value(Json::arrayValue);
            (*currentKontenklasse)["subClasses"].append(twoDigits);
            //currentKontenklasse->["subClasses"].append(twoDigits);
            current2Digits = &((*currentKontenklasse)["subClasses"][(*currentKontenklasse)["subClasses"].size() - 1]);
        } else if (item.type == "3Digits") {
            writeToLog("Handling a 3Digits");
            Json::Value threeDigits(Json::objectValue);
            threeDigits["name"] = item.name;
            threeDigits["entries"] = Json::Value(Json::arrayValue);

            std::string upperLimit = "999";  // default to the highest possible value
            if (idx + 1 < structure.size() && structure[idx+1].type == "3Digits") {
                upperLimit = extractFirst3Digits(structure[idx+1].name);
            }

            std::string currentStart = extractFirst3Digits(item.name);

            while (accountIt != accountsWithBalances.end() && currentStart <= accountIt->first && accountIt->first < upperLimit) {
                Json::Value account(Json::objectValue);
                writeToLog("balanceReport, iterateThroughAccounts"+accountIt->first);
                account["account"] = accountIt->first;
                account["sollSaldo"] = accountIt->second.sollSaldo;
                account["habenSaldo"] = accountIt->second.habenSaldo;

                threeDigits["entries"].append(account);
                accountIt++;
            }

            //current2Digits->["subClasses"].append(threeDigits);
            (*current2Digits)["subClasses"].append(threeDigits);
        }
        writeToLog("balanceReport, iterateThroughAccounts"+idx);
    }


    writeToLog("balanceReport, iterateThroughAccounts, before return");

    return root;
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
    std::ifstream inputFile("/fibudata/bookings.json");
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

    writeToLog("account: before computeBalanceForAccounts");
    auto balances = computeBalanceForAccounts(data, client);

    // Read the structure from file
    std::ifstream inFile("/app/EinheitskontenrahmenOE.txt");
    std::vector<StructureItem> structure;
    std::string line;

    while (std::getline(inFile, line)) {
        StructureItem item;
        item.name = line;

        if (line.substr(0, 12) == "Kontenklasse") {
            item.type = "Kontenklasse";
        } else if (isdigit(line[0]) && isdigit(line[1]) && !isdigit(line[2])) {
            item.type = "2Digits";
        } else if (isdigit(line[0]) && isdigit(line[1]) && isdigit(line[2])) {
            item.type = "3Digits";
        }

        structure.push_back(item);
    }
    inFile.close();

    // Iterate through the structure
    writeToLog("account: before iterateThroughAccounts. structureSize: "/* + structure.size()*/);


    Json::Value result = iterateThroughAccounts(structure, balances);

    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "    ";  // Use 4 spaces for indentation
    std::cout << "Content-Type: application/json\r\n\r\n";
    std::cout << Json::writeString(wbuilder, result) << std::endl;

    writeToLog("account: before return from balanceReport");
    return 0;
}


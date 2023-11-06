
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
#include <cstdlib>
#include <map>
#include <regex>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include "hash.h"
#include "logger.h"



void writeToLog(const std::string &message) {
    std::ofstream logFile("/fibudata/app_log.txt", std::ios_base::app);  // appending to the log file
    if (logFile.is_open()) {
        logFile << getCurrentTimeStamp() << " - " << message << std::endl;
        logFile.close();
    }
}



std::string getLastHash(std::string& client) {
    std::ifstream infile("/fibudata/"+client+"-bookingsHash");
    std::string hash;
    if (!infile) {
        // No existing hash file, start with a default value or create an initial block
        hash = "initial hash value"; // This should be the hash of an initial block
    } else {
        std::getline(infile, hash); // Assume each file contains a single hash
    }
    return hash;
}

void saveNewHash(const std::string& hash, std::string client) {
    std::ofstream outfile("/fibudata/"+client+"-bookingsHash");
    outfile << hash;
}

void addHash(Json::Value& booking, std::string& client) {
    std::string lastHash = getLastHash(client);
    booking["previousHash"] = lastHash;

    // Serialize the booking to a string and calculate its hash
    Json::StreamWriterBuilder builder;
    const std::string bookingData = Json::writeString(builder, booking);
    writeToLog("addHash - bookingData :" + bookingData);
    std::string newHash = calculateSHA256(bookingData);

    // Save the new hash for the next block
    saveNewHash(newHash, client);
}



int appendToJsonArrayFile(const std::string& filename, std::string& client, Json::Value& receivedData) {

    int fd = open(filename.c_str(), O_RDWR | O_CREAT, 0666); // Open or create file with rw-rw-rw- permissions
    if (fd == -1) {
        std::cerr << "Failed to open the file!" << std::endl;
        return 0;
    }

    // Lock the file
    struct flock fl;
    fl.l_type   = F_WRLCK;  // F_RDLCK, F_WRLCK, F_UNLCK
    fl.l_whence = SEEK_SET; // SEEK_SET, SEEK_CUR, SEEK_END
    fl.l_start  = 0;        // Offset from l_whence
    fl.l_len    = 0;        // length, 0 = to EOF
    fl.l_pid    = getpid(); // our PID

    if (fcntl(fd, F_SETLKW, &fl) == -1) {
        std::cerr << "Failed to lock the file!" << std::endl;
        close(fd);
        return 0;
    }

    addHash(receivedData, client);
    const std::string new_json_data = receivedData.toStyledString();
    writeToLog("appendToJsonArrayFilei: " + new_json_data);
    std::fstream file;
    file.open(filename, std::ios::in | std::ios::out | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open the file with fstream!" << std::endl;
        return 0;
    }

    // Check if the file is empty
    if (file.tellg() == 0) {
        // If the file is empty, initialize it with the new JSON data enclosed in square brackets
        file << "[\n" << new_json_data << "\n]";
    } else {
        // If the file is not empty, move two characters backward (to overwrite the closing bracket)
        file.seekp(-2, std::ios::end);

        // Append a comma, the new JSON data, and then the closing bracket
        file << ",\n" << new_json_data << "\n]";
    }

    file.close();

    // Unlock the file
    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        std::cerr << "Failed to unlock the file!" << std::endl;
    }

    close(fd);

    return 1;
}

void validateEntry(const Json::Value& entry) {
    // Define regex patterns for date and monetary amount
    std::regex datePattern(R"(^\d{4}-\d{2}-\d{2}$)"); // YYYY-MM-DD format
//    std::regex amountPattern(R"(^\d+(\.\d{2})?$)"); // Numeric with two decimal places
    std::regex amountPattern(R"(^-?\d+(\.\d{2})?$)");


    // List of required keys
    const std::vector<std::string> requiredKeys = {
        "Belegdatum", "Belegnummer", "Mandant", "Text", "SollKonto", "HabenKonto", "Betrag"
    };

    for (const auto& key : requiredKeys) {
        // Check if the key exists and is not null
        if (!entry.isMember(key) || entry[key].isNull()) {
            throw std::invalid_argument("Der Schlüssel '" + key + "' fehlt oder ist null.");
        }
        // Convert the JSON value to string and check if it's empty
        std::string value = entry[key].asString();
        if (value.empty()) {
            throw std::invalid_argument("Der Wert für '" + key + "' ist leer.");
        }
        // Validate date format
        if (key == "Belegdatum" && !std::regex_match(value, datePattern)) {
            throw std::invalid_argument("Das Datum '" + value + "' hat ein ungültiges Format. Erwartet: JJJJ-MM-TT.");
        }
        // Validate monetary amount format
        if (key == "Betrag" && !std::regex_match(value, amountPattern)) {
            throw std::invalid_argument("Der Betrag '" + value + "' hat ein ungültiges Format. Erwartet: Zahl mit zwei Dezimalstellen.");
        }
    }
}

int main() {
    Logger log("/fibudata/app_log.txt");

    writeToLog("addBooking");
    std::cout << "Access-Control-Allow-Origin: *" << std::endl;  // Allows any origin to access
    std::cout << "Access-Control-Allow-Methods: POST, GET, OPTIONS" << std::endl; // Allowed methods
    std::cout << "Access-Control-Allow-Headers: Content-Type" << std::endl;  // Allowed headers

    // Do not skip whitespace, more configuration may also be needed.
    std::cin >> std::noskipws;

    // Copy all data from cin, using iterators.
    std::istream_iterator<char> begin(std::cin);
    std::istream_iterator<char> end;
    std::string inputData(begin, end);
    // std::getline(std::cin, inputData);

    const char* contentType = getenv("CONTENT_TYPE");
    if (!contentType || std::string(contentType) != "application/json") {
        writeToLog("Invalid content type. Expected application/json.");
        std::string errorResponse = "{\"status\": \"error\", \"message\": \"Invalid content type. Expected application/json.\"}";
        std::cout << "Content-Length: " << errorResponse.length() << std::endl;
        std::cout << "Content-type: application/json" << std::endl << std::endl;
        std::cout << errorResponse;
        std::cout.flush();
        return 0;
    }

    try {
        Json::Value receivedData;
        Json::CharReaderBuilder builder;
        std::string errors;
        std::istringstream iss(inputData);
        if (!Json::parseFromStream(builder, iss, &receivedData, &errors)) {
            throw std::runtime_error(errors);
        }

        validateEntry(receivedData);

        std::string client = receivedData["Mandant"].asString();

        // Add additional fields
        receivedData["TimeStamp"] = getCurrentTimeStamp();
        const char* username = std::getenv("REMOTE_USER");
        if (!username) {
            username = std::getenv("HTTP_REMOTE_USER");
        }   
        if (username) {
            writeToLog (username);
            receivedData["User"] = username; // Change this as per your requirements
        } else {
            receivedData["User"] = "";
        }
       

        const char * vars[] = { "REMOTE_USER", "REMOTE_ADDR", "AUTH_TYPE", nullptr };

        for (int i = 0; vars[i]; ++i) {
            const char * value = getenv(vars[i]);
            if (value) {
                log << getCurrentTimeStamp() << " - " << vars[i] << ": " << value << std::endl;
            } else {
                log << getCurrentTimeStamp() << " - " << vars[i] << " is not set" << std::endl;
            }
        }

        // Append the data to bookings.json
        std::ofstream file("/fibudata/"+client+"-bookings.json", std::ios_base::app);
        if(appendToJsonArrayFile("/fibudata/"+client+"-bookings.json", client, receivedData)){
            std::string responseBody = "{\"status\": \"success\", \"message\": \"Booking added successfully!\"}";
            std::cout << "Content-Length: " << responseBody.size() << "\r\n";
            std::cout << "Content-type: application/json; charset=utf-8\r\n";
            std::cout << "Access-Control-Allow-Origin: *\r\n";  // Allows any origin to access
            std::cout << "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"; // Allowed methods
            std::cout << "Access-Control-Allow-Headers: Content-Type\r\n";  // Allowed headers
            std::cout << "Connection: close\r\n\r\n";
            std::cout << responseBody;
            writeToLog("addBooking: after responseBody");
        } else {
            std::cout << "{\"status\": \"error\", \"message\": \"Failed to open the bookings file!\"}" << std::endl;
        }
    } catch (const std::exception& e) {
        std::string errorResponse = "{\"status\": \"error\", \"message\": \"" + std::string(e.what()) + "\"}";
        std::cout << "Status: 400 Bad Request" << std::endl; // Send the HTTP status code
        std::cout << "Content-Type: application/json" << std::endl;
        std::cout << "Content-Length: " << errorResponse.length() << std::endl << std::endl;
        std::cout << errorResponse;
    }

    // After sending the JSON response
    std::cout.flush();
    writeToLog("before return");
    return 0;
}

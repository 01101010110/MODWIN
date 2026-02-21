#pragma once
#include <string>
#include "sqlite3.h"
#include <vector>

// Holds the data pulled from the knowledge base
struct AppKnowledge {
    std::string Description;
    int SafetyRating; 
    std::string Category;
};

// Manages the connection to the SQLite database
class DatabaseManager {
private:
    sqlite3* db;
    std::string dbPath;
    bool isConnected;

    // Checks if the database is new and needs default data
    void CheckAndPopulateDefaults();

public:
    DatabaseManager();
    ~DatabaseManager();

    // Opens the connection and creates tables if missing
    bool Initialize(std::string basePath);

    // Looks up a package name to see if the database knows what it is
    AppKnowledge GetAppInfo(std::string packageName);

    // Records an action into the history log
    void LogRemoval(std::string packageName, std::string type);
};

// Global instance defined in database.cpp
extern DatabaseManager g_dbManager;
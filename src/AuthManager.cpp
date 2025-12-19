#include "AuthManager.h"
#include "QueryLoader.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
using namespace std;
using namespace pqxx;
string AuthManager::hashPassword(const string& password) {
    unsigned long hash = 5381;
    for (char c : password) {
        hash = ((hash << 5) + hash) + c;
    }
    stringstream ss;
    ss << hex << hash;
    return ss.str();
}
AuthManager::AuthManager() : connected(false) {
    const char* env_host = getenv("DB_HOST");
    const char* env_port = getenv("DB_PORT");
    const char* env_name = getenv("DB_NAME");
    const char* env_user = getenv("DB_USER");
    const char* env_pass = getenv("DB_PASSWORD");
    string host = env_host ? env_host : "localhost";
    string port = env_port ? env_port : "5433";
    string dbname = env_name ? env_name : "nbody_db";
    string user = env_user ? env_user : "nbody_user";
    string password = env_pass ? env_pass : "";
    connectionString = "dbname=" + dbname + " user=" + user + " host=" + host + " port=" + port;
    if (!password.empty()) {
        connectionString += " password=" + password;
    }
}
AuthManager::~AuthManager() {
}
bool AuthManager::connect() {
    try {
        conn = make_unique<connection>(connectionString);
        if (conn->is_open()) {
            cout << "Connected to database successfully: " << conn->dbname() << endl;
            work txn(*conn);
            txn.exec(QueryLoader::getQuery("cleanup_duplicate_users"));
            txn.commit();
            connected = true;
            return true;
        }
        return false;
    } catch (const exception &e) {
        cerr << "Database connection error: " << e.what() << endl;
        return false;
    }
}
bool AuthManager::registerUser(const string& username, const string& password) {
    if (!connected) return false;
    try {
        work W(*conn);
        string hash = hashPassword(password);
        result r = W.exec_params(QueryLoader::getQuery("check_user_exists"), username);
        if (!r.empty()) return false;
        W.exec_params(QueryLoader::getQuery("insert_user"), username, hash);
        W.commit();
        return true;
    } catch (const exception &e) {
        cerr << "Registration error: " << e.what() << endl;
        return false;
    }
}
bool AuthManager::loginUser(const string& username, const string& password) {
    if (!connected) return false;
    try {
        work W(*conn);
        string hash = hashPassword(password);
        result r = W.exec_params(QueryLoader::getQuery("select_user_by_username"), username);
        if (r.empty()) return false;
        string storedHash = r[0][0].c_str();
        return storedHash == hash;
    } catch (const exception &e) {
        cerr << "Login error: " << e.what() << endl;
        return false;
    }
}
bool AuthManager::isConnected() const {
    return connected;
}
bool AuthManager::deleteUser(const string& username) {
    if (!connected || !conn) return false;
    try {
        work txn(*conn);
        result r = txn.exec_params(QueryLoader::getQuery("delete_user_by_username"), username);
        txn.commit();
        return r.affected_rows() > 0;
    } catch (const exception& e) {
        cerr << "Delete user error: " << e.what() << endl;
        return false;
    }
}
bool AuthManager::isAdmin(const string& username) {
    if (!connected || !conn) return false;
    try {
        work txn(*conn);
        result r = txn.exec_params(QueryLoader::getQuery("check_is_admin"), username);
        if (r.empty()) return false;
        if (r[0][0].is_null()) return false;
        return r[0][0].as<bool>();
    } catch (const exception& e) {
        return false;
    }
}
int AuthManager::getUserId(const string& username) {
    if (!connected || !conn) return -1;
    try {
        work txn(*conn);
        result r = txn.exec_params(QueryLoader::getQuery("check_user_exists"), username);
        if (r.empty()) return -1;
        return r[0][0].as<int>();
    } catch (const exception& e) {
        return -1;
    }
}
bool AuthManager::setAdminStatus(int id, bool isAdmin) {
    if (!connected || !conn) return false;
    try {
        work txn(*conn);
        txn.exec_params(QueryLoader::getQuery("update_user_admin_status"), isAdmin, id);
        txn.commit();
        return true;
    } catch (const exception& e) {
        return false;
    }
}
vector<User> AuthManager::getAllUsers() {
    vector<User> users;
    if (!connected || !conn) return users;
    try {
        work txn(*conn);
        result r = txn.exec(QueryLoader::getQuery("select_all_users"));
        for (const auto& row : r) {
            User user;
            user.id = row["id"].as<int>();
            user.username = row["username"].as<string>();
            user.is_admin = row["is_admin"].as<bool>();
            user.created_at = row["created_at"].as<string>();
            users.push_back(user);
        }
    } catch (const exception& e) {
        cerr << "Get all users error: " << e.what() << endl;
    }
    return users;
}
bool AuthManager::deleteUser(int id) {
    if (!connected || !conn) return false;
    try {
        work txn(*conn);
        result r = txn.exec_params(QueryLoader::getQuery("delete_user_by_id"), id);
        txn.commit();
        return r.affected_rows() > 0;
    } catch (const exception& e) {
        cerr << "Delete user error: " << e.what() << endl;
        return false;
    }
}
bool AuthManager::updateUsername(int id, const string& newUsername) {
    if (!connected || !conn) return false;
    try {
        work txn(*conn);
        txn.exec_params(QueryLoader::getQuery("update_username"), newUsername, id);
        txn.commit();
        return true;
    } catch (const exception& e) {
        cerr << "Update username error: " << e.what() << endl;
        return false;
    }
}
bool AuthManager::changePassword(const string& username, const string& oldPassword, const string& newPassword) {
    if (!connected || !conn) return false;
    if (!loginUser(username, oldPassword)) {
        return false;
    }
    try {
        string hashedPassword = hashPassword(newPassword);
        work txn(*conn);
        txn.exec_params(QueryLoader::getQuery("update_password"), hashedPassword, username);
        txn.commit();
        return true;
    } catch (const exception& e) {
        return false;
    }
}
void AuthManager::disconnect() {
    if (conn) {
        cout << "Disconnecting from database..." << endl;
        conn.reset();
        connected = false;
        cout << "Database disconnected." << endl;
    }
}

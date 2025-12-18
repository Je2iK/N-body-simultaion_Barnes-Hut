#include "AuthManager.h"
#include <iostream>
#include <sstream>
#include <iomanip>

#include <cstdlib>

using namespace std;
using namespace pqxx;

// Простая хеш-функция (для демонстрации, в реальном проекте используйте SHA256/bcrypt)
// hash не криптографически стойкая, но сойдет для примера без OpenSSL
string AuthManager::hashPassword(const string& password) {
    unsigned long hash = 5381;
    for (char c : password) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    stringstream ss;
    ss << hex << hash;
    return ss.str();
}

AuthManager::AuthManager() : connected(false) {
    // Получаем параметры подключения из переменных окружения.
    // Пароль передается драйверу (libpqxx) в открытом виде, так как драйвер сам занимается
    // хешированием при выполнении протокола аутентификации (MD5 или SCRAM-SHA-256).
    // Хардкодить пароли в коде — плохая практика, поэтому используем переменные окружения.
    
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
            txn.exec("ALTER TABLE users ADD COLUMN IF NOT EXISTS is_admin BOOLEAN DEFAULT FALSE");
            
            // Create benchmark_results table
            txn.exec(R"(
                CREATE TABLE IF NOT EXISTS benchmark_results (
                    id SERIAL PRIMARY KEY,
                    user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
                    algorithm_name VARCHAR(50) NOT NULL,
                    particle_count INTEGER NOT NULL,
                    steps INTEGER NOT NULL,
                    duration_ms INTEGER NOT NULL,
                    fps_equivalent DOUBLE PRECISION NOT NULL,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                )
            )");

            // real_password column removed for security
            
            // Remove duplicates if any exist (keep oldest)
            try {
                txn.exec("DELETE FROM users a USING users b WHERE a.id > b.id AND a.username = b.username");
            } catch (...) {}

            string adminHash = hashPassword("admin");
            string query = "INSERT INTO users (username, password_hash, is_admin) VALUES ('admin', '" + adminHash + "', TRUE) ON CONFLICT (username) DO UPDATE SET is_admin = TRUE";
            txn.exec(query);
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
        
        result r = W.exec_params("SELECT id FROM users WHERE username = $1", username);
        if (!r.empty()) return false;
        
        W.exec_params("INSERT INTO users (username, password_hash) VALUES ($1, $2)", username, hash);
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
        
        result r = W.exec_params("SELECT password_hash FROM users WHERE username = $1", username);
        
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
        
        string query = "DELETE FROM users WHERE username = " + txn.quote(username);
        result r = txn.exec(query);
        
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
        string query = "SELECT is_admin FROM users WHERE username = " + txn.quote(username);
        result r = txn.exec(query);
        
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
        string query = "SELECT id FROM users WHERE username = " + txn.quote(username);
        result r = txn.exec(query);
        
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
        string query = "UPDATE users SET is_admin = " + string(isAdmin ? "TRUE" : "FALSE") + " WHERE id = " + to_string(id);
        txn.exec(query);
        txn.commit();
        return true;
    } catch (const exception& e) {
        return false;
    }
}

string AuthManager::getPasswordHash(int id) {
    if (!connected || !conn) return "";
    try {
        work txn(*conn);
        string query = "SELECT password_hash FROM users WHERE id = " + to_string(id);
        result r = txn.exec(query);
        if (r.empty()) return "";
        return r[0][0].as<string>();
    } catch (const exception& e) {
        return "";
    }
}

vector<AuthManager::User> AuthManager::getAllUsers() {
    vector<User> users;
    if (!connected || !conn) return users;
    
    try {
        work txn(*conn);
        string query = "SELECT id, username, is_admin, created_at FROM users ORDER BY id";
        result r = txn.exec(query);
        
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
        string query = "DELETE FROM users WHERE id = " + to_string(id);
        result r = txn.exec(query);
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
        string query = "UPDATE users SET username = " + txn.quote(newUsername) + " WHERE id = " + to_string(id);
        txn.exec(query);
        txn.commit();
        return true;
    } catch (const exception& e) {
        cerr << "Update username error: " << e.what() << endl;
        return false;
    }
}

bool AuthManager::updatePassword(int id, const string& newPassword) {
    if (!connected || !conn) return false;
    try {
        string hashedPassword = hashPassword(newPassword);
        work txn(*conn);
        string query = "UPDATE users SET password_hash = " + txn.quote(hashedPassword) + 
                           " WHERE id = " + to_string(id);
        txn.exec(query);
        txn.commit();
        return true;
    } catch (const exception& e) {
        cerr << "Update password error: " << e.what() << endl;
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

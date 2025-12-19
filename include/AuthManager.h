#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include <string>
#include <optional>
#include <vector>
#include <pqxx/pqxx> 

using namespace std;
using namespace pqxx; 

class AuthManager {
public:
    AuthManager();
    ~AuthManager();

    struct User {
        int id;
        string username;
        bool is_admin;
        string created_at;
    };

    bool connect();
    void disconnect(); // Added this line
    bool registerUser(const string& username, const string& password);
    bool loginUser(const string& username, const string& password);
    bool deleteUser(const string& username);
    bool deleteUser(int id);
    bool updateUsername(int id, const string& newUsername);
    bool changePassword(const string& username, const string& oldPassword, const string& newPassword);
    bool isAdmin(const string& username);
    int getUserId(const string& username);
    bool setAdminStatus(int id, bool isAdmin);
    vector<User> getAllUsers();
    bool isConnected() const;

private:
    string connectionString;
    bool connected;
    
    unique_ptr<connection> conn;
    
    string hashPassword(const string& password);
};

#endif // AUTH_MANAGER_H

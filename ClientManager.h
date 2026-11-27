#pragma once 
#include <string>
#include <pqxx/pqxx>

using namespace std;

class ClientManager {
public:

	explicit ClientManager(const string& conn_str);
	void createTables();
	int addClient(const string& first, const string& last, const string& email);
	void addPhone(int client_id, const string& phone);
	void updateClient(int id, const string& first, const string& last, const string&
		email);
	void deletePhone(int client_id, const string& phone);
	void deleteClient(int id);
	vector<int> findClient(const string& q);
	void listAll();
private:
	pqxx::connection conn; 
	void prepareStatements(); 
};

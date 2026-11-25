#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "ClientManager.h"
#include <iostream>

void printClientRow(const pqxx::row_ref& row) {
	cout << " id=" << row["id"].as<int>()
		<< " | " << row["first_name"].as<string>()
		<< " " << row["last_name"].as<string>()
		<< " | " << row["email"].as<string>()
		<< " | тел.: " << row["phones"].as<string>()
		<< "\n";
}
ClientManager::ClientManager(const string& conn_str) : conn(conn_str) {

}
void ClientManager::createTables() {
	pqxx::work tx{ conn };
	tx.exec(
		"CREATE TABLE IF NOT EXISTS clients ("
		" id SERIAL PRIMARY KEY,"
		" first_name VARCHAR(100) NOT NULL,"
		" last_name VARCHAR(100) NOT NULL,"
		" email VARCHAR(200) UNIQUE NOT NULL"
		");"
		"CREATE TABLE IF NOT EXISTS phones ("
		" id SERIAL PRIMARY KEY,"
		" client_id INTEGER NOT NULL REFERENCES clients(id) ON DELETE CASCADE,"
		" phone VARCHAR(30) NOT NULL,"
		" UNIQUE(client_id, phone)"
		");"
	);
	tx.commit();
	cout << "[OK] Таблицы clients и phones готовы.\n";
	prepareStatements();
}
int ClientManager::addClient(const string& first, const string& last, const string& email) {
	pqxx::work tx{ conn };

	auto row = tx.exec(pqxx::prepped{ "ins_client" }, pqxx::params{ first, last, email }); 
	tx.commit();
	return row[0][0].as<int>();
}
void ClientManager::addPhone(int client_id, const string& phone) {
	pqxx::work tx{ conn };
	tx.exec(pqxx::prepped{ "ins_phone" }, pqxx::params{ client_id, phone }); 
	tx.commit();
}
void ClientManager::updateClient(int id, const string& first, const string& last, const string& email) {
	pqxx::work tx{ conn };
	tx.exec(pqxx::prepped{ "upd_client" }, pqxx::params{ first, last, email, id }); 
	tx.commit();
}
void ClientManager::deletePhone(int client_id, const string& phone) {
	pqxx::work tx{ conn };
	tx.exec(pqxx::prepped{ "del_phone" }, pqxx::params( client_id, phone)); 
	tx.commit();
}
void ClientManager::deleteClient(int id) {
	pqxx::work tx{ conn };
	tx.exec(pqxx::prepped{ "del_client" }, id); 
	tx.commit();
}
void ClientManager::findClient(const string& q) {
	pqxx::work tx{ conn };
	string pattern = "%" + q + "%";
	auto res = tx.exec(pqxx::prepped{ "find_client" }, pqxx::params(pattern, pattern, pattern, pattern)); 
		if (res.empty()) {
			cout << "[SEARCH] Ничего не найдено по запросу: " << q << "\n";
			return;
		}
	cout << "[SEARCH] Найдено " << res.size() << " клиент(ов) по запросу \"" << q << "\":\n";
	for (auto row : res) {
		printClientRow(row);
	}
}
void ClientManager::listAll() {
	pqxx::work tx{ conn };
	auto res = tx.exec(pqxx::prepped{ "list_all" }); 
	cout << "[LIST] Всего клиентов: " << res.size() << "\n";
	for (auto row : res) {
		printClientRow(row);
	}
}
void ClientManager::prepareStatements() {
	conn.prepare("ins_client", "INSERT INTO clients(first_name, last_name, email) VALUES($1, $2, $3) RETURNING id");
		conn.prepare("ins_phone", "INSERT INTO phones(client_id, phone) VALUES($1, $2)");
	conn.prepare("upd_client", "UPDATE clients SET first_name=$1, last_name=$2, email=$3 WHERE id= $4");
		conn.prepare("del_phone", "DELETE FROM phones WHERE client_id=$1 AND phone=$2");
	conn.prepare("del_client", "DELETE FROM clients WHERE id=$1");
	conn.prepare(
		"find_client",
		"SELECT c.id, c.first_name, c.last_name, c.email, COALESCE(string_agg(p.phone, ', '), '—') AS phones "
		"FROM clients c LEFT JOIN phones p ON p.client_id = c.id "
		"WHERE c.first_name ILIKE $1 OR c.last_name ILIKE $2 OR c.email ILIKE $3 "
		" OR c.id IN (SELECT client_id FROM phones WHERE phone ILIKE $4) "
		"GROUP BY c.id, c.first_name, c.last_name, c.email ORDER BY c.id"
	);
	conn.prepare(
		"list_all",
		"SELECT c.id, c.first_name, c.last_name, c.email, COALESCE(string_agg(p.phone, ', '), '—') AS phones "
		"FROM clients c LEFT JOIN phones p ON p.client_id = c.id "
		"GROUP BY c.id, c.first_name, c.last_name, c.email ORDER BY c.id"
	);
}

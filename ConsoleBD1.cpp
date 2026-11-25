
#include <iostream>
#include <pqxx/pqxx>
#include <windows.h>
#include "ClientManager.h" 

int main() {
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	const string conn_str =
		"host=localhost "
		"port=5432 "
		"dbname=clients "
		"user=postgres "
		"password=505nika505 "
		"client_encoding=WIN1251";
	try {
		ClientManager mgr(conn_str); 
		mgr.createTables();
		int id1 = mgr.addClient("Соня", "Симонян", "simonyan@mail.ru");
		int id2 = mgr.addClient("Ваня", "Симонян", "gambaryan@mail.ru");
		cout << "[OK] Добавлены клиенты с id: " << id1 << ", " << id2 << "\n";

		mgr.addPhone(id1, "+7-911-111-11-11");
		mgr.addPhone(id1, "+7-922-222-22-22");
		mgr.addPhone(id2, "+7-933-333-33-33");
		mgr.listAll(); 

		mgr.updateClient(id2, "Ваня", "Петросян", "petrosyan@mail.ru");
		cout << "[OK] Данные клиента id=" << id2 << " обновлены.\n";
		mgr.listAll();

		mgr.deletePhone(id1, "+7-922-222-22-22");
		cout << "[OK] Один телефон у id=" << id1 << " удалён.\n";
		mgr.listAll();

		mgr.findClient("Симонян");
		mgr.findClient("222"); 

		mgr.deleteClient(id2);
		cout << "[OK] Клиент id=" << id2 << " удалён.\n";
		mgr.listAll();
	}
	catch (const pqxx::sql_error& e) {
		cerr << "[SQL ERROR] " << e.what() << "\n";
		return 1;
	}
	catch (const exception& e) {
		cerr << "[ERROR] " << e.what() << "\n";
		return 1;
	}
	return 0;
}

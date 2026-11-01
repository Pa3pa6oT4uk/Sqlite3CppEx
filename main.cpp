#include <windows.h>
#include <iostream>
#include <string>
#include <limits>

#include "sqlite3.h"

constexpr const char* DATABASE_NAME = "database.db";

static sqlite3* db = nullptr;

// Инициализация базы данных
static bool database_init()
{
    int result = sqlite3_open(DATABASE_NAME, &db);

    if (result != SQLITE_OK)
    {
        std::cerr << "Ошибка открытия БД: "
                  << sqlite3_errmsg(db) << '\n';

        return false;
    }

    const char* sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "age INTEGER NOT NULL"
        ");";

    char* error_message = nullptr;

    result = sqlite3_exec(db, sql, nullptr, nullptr, &error_message);

    if (result != SQLITE_OK)
    {
        std::cerr << "Ошибка создания таблицы: "
                  << error_message << '\n';

        sqlite3_free(error_message);
        return false;
    }

    return true;
}

// Вспомогательная функция для очистки ввода
static void clear_input()
{
    std::cin.clear();
    // std::cin.ignore(
    //     std::numeric_limits<std::streamsize>::max(),
    //     '\n'
    // );
}

// CREATE: Добавление пользователя
static void user_create()
{
    sqlite3_stmt* stmt = nullptr;

    const char* sql =
        "INSERT INTO users (name, age) VALUES (?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Ошибка подготовки запроса: "
                  << sqlite3_errmsg(db) << '\n';
        return;
    }

    std::string name;
    int age;

    std::cout << "Введите имя: ";
    std::getline(std::cin, name);

    std::cout << "Введите возраст: ";

    if (!(std::cin >> age) || age < 0)
    {
        std::cout << "Некорректный возраст!\n";
        clear_input();
        sqlite3_finalize(stmt);
        return;
    }

    clear_input();

    sqlite3_bind_text(
        stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT
    );

    sqlite3_bind_int(stmt, 2, age);

    if (sqlite3_step(stmt) == SQLITE_DONE)
    {
        std::cout << "Пользователь добавлен! ID: "
                  << sqlite3_last_insert_rowid(db) << '\n';
    }
    else
    {
        std::cerr << "Ошибка добавления: "
                  << sqlite3_errmsg(db) << '\n';
    }

    sqlite3_finalize(stmt);
}

// READ: Получение всех пользователей
static void user_read_all()
{
    sqlite3_stmt* stmt = nullptr;

    const char* sql =
        "SELECT id, name, age FROM users ORDER BY id;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Ошибка запроса: "
                  << sqlite3_errmsg(db) << '\n';
        return;
    }

    std::cout << "\n========== USERS ==========\n";

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);

        const unsigned char* name =
            sqlite3_column_text(stmt, 1);

        int age = sqlite3_column_int(stmt, 2);

        std::cout
            << "ID: " << id
            << " | Имя: " << (name ? reinterpret_cast<const char*>(name) : "")
            << " | Возраст: " << age
            << '\n';
    }

    std::cout << "===========================\n";

    sqlite3_finalize(stmt);
}

// READ: Поиск пользователя по ID
static void user_read_by_id()
{
    sqlite3_stmt* stmt = nullptr;

    int id;

    std::cout << "Введите ID пользователя: ";

    if (!(std::cin >> id) || id <= 0)
    {
        std::cout << "Некорректный ID!\n";
        clear_input();
        return;
    }

    clear_input();

    const char* sql =
        "SELECT id, name, age FROM users WHERE id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Ошибка запроса: "
                  << sqlite3_errmsg(db) << '\n';
        return;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::cout << "\nID: "
                  << sqlite3_column_int(stmt, 0) << '\n';

        const unsigned char* name =
            sqlite3_column_text(stmt, 1);

        std::cout << "Имя: "
                  << (name ? reinterpret_cast<const char*>(name) : "")
                  << '\n';

        std::cout << "Возраст: "
                  << sqlite3_column_int(stmt, 2) << '\n';
    }
    else
    {
        std::cout << "Пользователь не найден!\n";
    }

    sqlite3_finalize(stmt);
}

// UPDATE: Изменение пользователя
static void user_update()
{
    sqlite3_stmt* stmt = nullptr;

    int id;
    int age;
    std::string name;

    std::cout << "Введите ID пользователя: ";

    if (!(std::cin >> id) || id <= 0)
    {
        std::cout << "Некорректный ID!\n";
        clear_input();
        return;
    }

    clear_input();

    std::cout << "Введите новое имя: ";
    std::getline(std::cin, name);

    std::cout << "Введите новый возраст: ";

    if (!(std::cin >> age) || age < 0)
    {
        std::cout << "Некорректный возраст!\n";
        clear_input();
        return;
    }

    clear_input();

    const char* sql =
        "UPDATE users SET name = ?, age = ? WHERE id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Ошибка запроса: "
                  << sqlite3_errmsg(db) << '\n';
        return;
    }

    sqlite3_bind_text(
        stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT
    );

    sqlite3_bind_int(stmt, 2, age);
    sqlite3_bind_int(stmt, 3, id);

    if (sqlite3_step(stmt) == SQLITE_DONE)
    {
        if (sqlite3_changes(db) > 0)
            std::cout << "Пользователь обновлён!\n";
        else
            std::cout << "Пользователь не найден!\n";
    }
    else
    {
        std::cerr << "Ошибка обновления: "
                  << sqlite3_errmsg(db) << '\n';
    }

    sqlite3_finalize(stmt);
}

// DELETE: Удаление пользователя
static void user_delete()
{
    sqlite3_stmt* stmt = nullptr;

    int id;

    std::cout << "Введите ID пользователя: ";

    if (!(std::cin >> id) || id <= 0)
    {
        std::cout << "Некорректный ID!\n";
        clear_input();
        return;
    }

    clear_input();

    const char* sql =
        "DELETE FROM users WHERE id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Ошибка запроса: "
                  << sqlite3_errmsg(db) << '\n';
        return;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_DONE)
    {
        if (sqlite3_changes(db) > 0)
            std::cout << "Пользователь удалён!\n";
        else
            std::cout << "Пользователь не найден!\n";
    }
    else
    {
        std::cerr << "Ошибка удаления: "
                  << sqlite3_errmsg(db) << '\n';
    }

    sqlite3_finalize(stmt);
}

// Главное меню
static void menu()
{
    int choice;

    while (true)
    {
        std::cout
            << "\n========== SQLITE3 CRUD ==========\n"
            << "1. Добавить пользователя\n"
            << "2. Показать всех пользователей\n"
            << "3. Найти пользователя по ID\n"
            << "4. Изменить пользователя\n"
            << "5. Удалить пользователя\n"
            << "0. Выход\n"
            << "==================================\n"
            << "Выберите действие: ";

        if (!(std::cin >> choice))
        {
            std::cout << "Некорректный ввод!\n";
            clear_input();
            continue;
        }

        clear_input();

        switch (choice)
        {
            case 1: user_create(); break;
            case 2: user_read_all(); break;
            case 3: user_read_by_id(); break;
            case 4: user_update(); break;
            case 5: user_delete(); break;
            case 0: return;
            default: std::cout << "Неизвестная команда!\n"; break;
        }
    }
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    if (!database_init())
    {
        if (db != nullptr)
            sqlite3_close(db);
        return EXIT_FAILURE;
    }

    std::cout << "SQLite3 успешно инициализирована!\n";
    menu();
    sqlite3_close(db);

    return EXIT_SUCCESS;
}
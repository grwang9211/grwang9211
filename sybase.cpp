// sybase_odbc_example.cpp
// Build: g++ -std=c++17 sybase_odbc_example.cpp -lodbc -o sybase_odbc_example
//
// Usage:
//   ./sybase_odbc_example "DRIVER=DataDirect 8.0 Sybase Wire Protocol;HOST=sybase_host;PORT=5000;DB=mydb;UID=myuser;PWD=mypass;"

#include <sql.h>
#include <sqlext.h>
#include <iostream>
#include <string>

void print_diag(SQLSMALLINT type, SQLHANDLE handle, const char* where) {
    SQLCHAR sqlstate[6], msg[256];
    SQLINTEGER native;
    SQLSMALLINT len;
    int i = 1;
    while (SQLGetDiagRec(type, handle, i++, sqlstate, &native, msg, sizeof(msg), &len) == SQL_SUCCESS) {
        std::cerr << "[ODBC] " << where << " SQLSTATE=" << sqlstate << " Native=" << native
                  << " Msg=" << msg << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " \"<connection-string>\"\n";
        return 1;
    }

    std::string conn_str = argv[1];

    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    SQLRETURN ret;

    // Allocate environment
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

    // Allocate connection
    SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);

    // Connect using DataDirect Sybase driver
    SQLCHAR outstr[1024];
    SQLSMALLINT outstrlen;
    ret = SQLDriverConnect(dbc, NULL,
                           (SQLCHAR*)conn_str.c_str(), SQL_NTS,
                           outstr, sizeof(outstr), &outstrlen,
                           SQL_DRIVER_NOPROMPT);
    if (!(SQL_SUCCEEDED(ret))) {
        print_diag(SQL_HANDLE_DBC, dbc, "SQLDriverConnect");
        return 1;
    }

    std::cout << "Connected: " << outstr << "\n";

    // Allocate statement
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

    // Run a simple query
    const char* sql = "SELECT @@servername, getdate()";
    ret = SQLExecDirect(stmt, (SQLCHAR*)sql, SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        print_diag(SQL_HANDLE_STMT, stmt, "SQLExecDirect");
    } else {
        SQLCHAR col1[128], col2[128];
        SQLLEN ind1, ind2;
        SQLBindCol(stmt, 1, SQL_C_CHAR, col1, sizeof(col1), &ind1);
        SQLBindCol(stmt, 2, SQL_C_CHAR, col2, sizeof(col2), &ind2);
        while (SQLFetch(stmt) == SQL_SUCCESS) {
            std::cout << "Server=" << col1 << " Time=" << col2 << "\n";
        }
    }

    // Cleanup
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);

    std::cout << "Disconnected.\n";
    return 0;
}


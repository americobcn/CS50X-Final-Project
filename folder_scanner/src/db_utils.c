#include "db_utils.h"


/* Functions Definitions */
int createDataBase(const char* DBName)
{
    sqlite3 *db;
    int rc;
    
    /* Open database */
    rc = sqlite3_open(DBName, &db);   
    if( rc != SQLITE_OK) {
        fprintf(stderr, "[%s:%d] db_utils: Can't open database: %s\n", __FILE__, __LINE__, sqlite3_errmsg(db));
        return EXIT_FAILURE;
    }
    
    char* sql = "CREATE TABLE IF NOT EXISTS projects ("
            "id INTEGER PRIMARY KEY, "
            "project TEXT, "
            "client TEXT, "
            "month TEXT, "
            "year INTEGER, "
            "path TEXT);"
            "CREATE INDEX IF NOT EXISTS idx_project ON projects(project);"
            "CREATE INDEX IF NOT EXISTS idx_client ON projects(client);"
            "CREATE INDEX IF NOT EXISTS idx_year ON projects(year);";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, NULL, 0, NULL);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "db_utils: Can't create table: %s\n", sqlite3_errmsg(db));
    } 

    sqlite3_close(db);
    
    return rc;
}



/********************************************************************************/
/* This function iterates the linked list inserting the nodes into the database.*/
/* update flag controls whether to delete current year existing records before  */
/* inserting                                                                    */
/********************************************************************************/
int insertData(node* head, const char* DBName, bool update)
{
    node* iter = head ? head->next : NULL;    
    if (iter == NULL || head->next == NULL)
    {
        return 0;
    }
    
    sqlite3* db;    

    /* Open database */
    int rc = sqlite3_open(DBName, &db);
    if( rc != SQLITE_OK) {
        fprintf(stderr, "db_utils: Can't open database: %s\n", sqlite3_errmsg(db));
        if (db) sqlite3_close(db);
        return 0;
    }

    /* BEGIN TRANSACTION */
    rc = sqlite3_exec(db, "BEGIN;", NULL, 0, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "db_utils: Cannot begin transaction: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    /* Delete all records to be updated */
    if (update) {
        const char* sql = "DELETE FROM projects WHERE year=?";
        sqlite3_stmt* stmt = NULL;
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        
        if (rc != SQLITE_OK) {
            fprintf(stderr, "db_utils: Failed to prepare delete statement: %s\n", sqlite3_errmsg(db));
            if (stmt) sqlite3_finalize(stmt);
            sqlite3_close(db);
            return 0;
        }

        sqlite3_bind_int(stmt, 1, getCurrentYear());
        rc = sqlite3_step(stmt);

        if (rc != SQLITE_DONE) {
            fprintf(stderr, "db_utils: delete error: %s\n", sqlite3_errmsg(db));            
            sqlite3_exec(db, "ROLLBACK;", NULL, 0, NULL);
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return 0;
        }
        sqlite3_finalize(stmt);                          
    }

    const char* sql = "INSERT INTO projects (project,client,month,year,path) VALUES (?,?,?,?,?)";
    sqlite3_stmt* stmt = NULL;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "db_utils: Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        if (stmt) sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    }

    /*  Insert linked list data into database using prepared statement */
    unsigned long accum = 0, nErrors = 0;
    while (iter != NULL)
    {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);

        sqlite3_bind_text(stmt, 1, iter->project, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, iter->client, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, iter->month, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, iter->year);
        sqlite3_bind_text(stmt, 5, iter->path, -1, SQLITE_TRANSIENT);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) 
        {
            nErrors++;
            fprintf(stderr, "db_utils: insert error: %s\n", sqlite3_errmsg(db));
            printNode(iter);
        } 
        else 
        {
            accum++;
        }

        iter = iter->next;
    }
    
    rc = sqlite3_exec(db, "COMMIT;", NULL, 0, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "db_utils: Cannot commit transaction: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return accum; // Still return what was inserted
    }

    printf("%lu records inserted successfully\n", accum);
    printf("%lu records error produced.\n", nErrors);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return accum;
}



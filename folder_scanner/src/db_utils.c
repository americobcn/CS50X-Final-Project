#include "db_utils.h"


/* Functions Definitions */
int createDataBase(const char* DBName)
{
    sqlite3 *db;
    int rc;
    
    /* Open database */
    rc = sqlite3_open(DBName, &db);
   
    if( rc != SQLITE_OK) {
        fprintf(stderr, "db_utils: Can't open database: %s\n", sqlite3_errmsg(db));
        return EXIT_FAILURE;
    } 
    
    char* sql = "CREATE TABLE IF NOT EXISTS projects (id INTEGER PRIMARY KEY, project TEXT, client TEXT, month TEXT, year INTEGER, path TEXT);";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, NULL, 0, NULL);

    if( rc != SQLITE_OK ){
        fprintf(stderr, "db_utils: Can't create table: %s\n", sqlite3_errmsg(db));
    } 


    sqlite3_close(db);
    
    return rc;
}



int insertData(node* head, const char* DBName)
{
    node* iter = head->next;
    
    sqlite3* db;
    int rc;

    /* Open database */
    rc = sqlite3_open(DBName, &db);
   
    if( rc != SQLITE_OK) {
        fprintf(stderr, "db_utils: Can't open database: %s\n", sqlite3_errmsg(db));
        return(0);
    }
    
    
    unsigned long accum = 0, nErrors = 0;
    /*  Construction of the insertion query */
    char query[512] = "INSERT INTO projects (project,client,month,year,path) VALUES (";
    while (iter != NULL)
    {
        char tmp[512];
        strcpy(tmp, query);
        strcat(tmp, "\'");
        strcat(tmp, iter->project);
        strcat(tmp, "\',\'");
        strcat(tmp, iter->client);
        strcat(tmp, "\',\'");
        strcat(tmp, iter->month);
        strcat(tmp, "\',\'");
        strcat(tmp, iter->year);
        strcat(tmp, "\',\'");
        strcat(tmp, iter->path);
        strcat(tmp, "\');\0");

        /* Execute the insertion query */
        rc = sqlite3_exec(db, tmp, NULL, 0, NULL);
        if( rc != SQLITE_OK )
        {
            nErrors++;
            fprintf(stderr, "db_utils: %s\n", sqlite3_errmsg(db));
            printNode(iter);            
        }
        
        accum++;
        iter = iter->next;
    }

    printf("%lu records inserted successfully\n", accum);
    printf("%lu records error produced.\n", nErrors);

    sqlite3_close(db);
    
    return accum;
    
}


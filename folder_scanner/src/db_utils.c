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



/********************************************************************************/
/* This function iterates the linked list inserting the nodes into the database */
/********************************************************************************/
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
    char *sqlBuffer; 
    while (iter != NULL)
    {
        int ret = asprintf(&sqlBuffer, "INSERT INTO projects (project,client,month,year,path) VALUES ('%s','%s','%s','%s','%s')",
                            iter->project, iter->client, iter->month, iter->year, iter->path);
        if (ret == -1) {
            printf("Couldn't create SQL Buffer for Inserion\n");
            free(sqlBuffer);
            exit(EXIT_FAILURE);
        }
    
        /* Execute the insertion query */
        // printf("SQL Buffer: %s\n", sqlBuffer);
        rc = sqlite3_exec(db, sqlBuffer, NULL, 0, NULL);
        if( rc != SQLITE_OK )
        {
            nErrors++;
            fprintf(stderr, "db_utils: %s\n", sqlite3_errmsg(db));
            printNode(iter);            
        }
        
        accum++;
        iter = iter->next;
        free(sqlBuffer);
    }

    printf("%lu records inserted successfully\n", accum);
    printf("%lu records error produced.\n", nErrors);
    
    sqlite3_close(db);
    
    return accum;
}

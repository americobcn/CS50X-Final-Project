/**********************************************************************************************
    folder_scanner: tool to scan a root folder path, parse the subfolders paths into a struct,
    store the struct as nodes in a linked list and finally persistenly store the linked list 
    into a single table Sqlite3 database.
    Usage: ./folder_scanner 'Volumes/root/path' subfolder_level '/Volumes/path/to/database.db'
    subfolder_level: int that represent the Nth element in the path that is the project name
    Path ex: /Volumes/MediaHD/backups/year/month/client/project/files&subfolders
                1       2       3     4     5      6      7
    Call ex: ./folder_scanner '/Volumes/MediaHD/backups' 7 '/Volumes/Users/user/data.db'

    Created by Américo Cot on 02/11/22.
***********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "reader_utils.h"
#include "db_utils.h"

#define MAX_FOLDERS_TO_SCAN 32

/* Global vars initialized in reader_utils.c */ 
extern node* head;
extern int *elementsToScan;


/* Main Function */
int main(int argc, char *argv[])
{
    /*  Check for valid arguments to perform the task */
    bool isValid = checkForValidArguments(argc, argv);
    if (isValid == false)
    {
        printf("Check arguments\n");
        return EXIT_FAILURE;
    }
    

    /* Var to store time used to calculate time consumed by the task */
    time_t start_t, end_t;
    time(&start_t);


    /* Init an empty linked list with a dummy head */
    head = new_node("");    // head is a global var declared in reader_utils.h
    if (head == NULL)  
    {
        printf("Couldn't initialize the linked list\n");
        return EXIT_FAILURE;
    }

    /* Load into foldersBuf the subfolders that will be scanned */
    char foldersBuf[MAX_FOLDERS_TO_SCAN][PATH_MAX];
    foldersToScan(argv, MAX_FOLDERS_TO_SCAN, PATH_MAX, foldersBuf);


    /* Start scanning foldersBuf and add to linked list */
    for (int i = 0; i < MAX_FOLDERS_TO_SCAN; i++)
    {
       scanPath(foldersBuf[i]);
    }
    
    /***     Database related section   ***/
    int rc;
    int recordsInserted = 0;


    /* Insert nodes form de linked list into the database */
    
    rc = createDataBase(argv[3]);
    recordsInserted = insertData(head, argv[3]);
    if (recordsInserted == 0)
    {
        printf("Not records inserted, something went wrong.\nQuiting ...\n");
        return EXIT_FAILURE;
    }
    

    /* End elapsed time */
    time(&end_t);
    printf("Elapsed seconds: %f\n", difftime(end_t, start_t));


    /* Clearing memory */
    clearList(head);


    /* Chek for leaks */
    // system("leaks folder_scanner");

    return 0;
    
    
}

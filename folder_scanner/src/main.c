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
    printf("Main started\n");
    /*  Check for valid arguments to perform the task */    
    if (checkForValidArguments(argc, argv) == false)
    {
        printf("Check arguments\n");
        exit(EXIT_FAILURE);
    }
    printf("Checked arguments\n");

    /* Store time to calculate time consumed by the task */
    time_t start_t, end_t;
    time(&start_t);
    printf("Timer seted\n");

    /* Init an empty linked list with a dummy head */
    printf("Initializing Linked List\n");
    head = new_node("");    // head is a global var declared in reader_utils.h
    if (head == NULL)  
    {
        printf("Couldn't initialize the linked list\n");
        exit(EXIT_FAILURE);
    }
    printf("Initialized Linked List\n");

    /* Load into foldersBuf the subfolders that will be scanned, one for each year */
    char foldersBuf[MAX_FOLDERS_TO_SCAN][PATH_MAX];
    memset(foldersBuf, 0, sizeof(foldersBuf));
    printf("Initialized foldersBuf\n");

    /* Determine and save which folders must be scanned */
    foldersToScan(argv, MAX_FOLDERS_TO_SCAN, PATH_MAX, foldersBuf);
    printf("Saved folders to be scanned\n");

    /* Start scanning foldersBuf and add to linked list */
    int i = 0;
    while (foldersBuf[i][0] != 0)
    {
        scanPath(foldersBuf[i]);
        i++;
    }
    
    /***     Database related section   ***/
    int rc;
    int recordsInserted = 0;
    /* Insert linked list into the database */
    
    rc = createDataBase(argv[3]);
    recordsInserted = insertData(head, argv[3]);
    if (recordsInserted == 0)
    {
        printf("Not records inserted, something went wrong.\nQuiting ...\n");
        return EXIT_FAILURE;
    }
    

    /* End elapsed time */
    time(&end_t);
    printf("Elapsed seconds: %.2f\n", difftime(end_t, start_t));


    /* Clearing memory */
    clearList(head);


    /* Chek for leaks while compiling with XCODE */
    // system("leaks folder_scanner");

    return 0;
        
}

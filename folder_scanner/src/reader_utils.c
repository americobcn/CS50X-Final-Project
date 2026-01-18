#include "reader_utils.h"
#include <time.h>

#define MAX_TOKENS 64

/* Globals vars used in main.c and db_utils.c */
node* head = NULL;
int elementsToScan = 0;


/* Functions definitions */
void addPathToList(node* head, const char* path)
{   
    node* n = new_node(path);
    n->next = head->next;
    head->next = n;
}


node* new_node(const char *path)
{
    node *n = malloc(sizeof(node));
    if (n == NULL)
    {
        fprintf(stderr, "ERROR: Could not allocate memory for new node\n");
        exit(EXIT_FAILURE);
    }
    
    n->next = NULL; 
    n->month[0] = '\0';
    n->client[0] = '\0';
    n->project[0] = '\0';
    n->path[0] = '\0';

    
    /* Are we creating an empty node as Head? */
    if (strcmp(path, "") == 0) {
        printf("Head created\n");
        return n;
    }

    
    /*  Need a buffer for strtok() function to parse the path */
    char lineBuffer[PATH_MAX];
    if (path != NULL)
    {
        snprintf(lineBuffer, sizeof(lineBuffer), "%s", path);
    }
    
    /* Calculate indexes of the tokens we are interested in */
    int yearIndex = elementsToScan - 4;
    int monthIndex = elementsToScan - 3;
    int clientIndex = elementsToScan - 2;
    int projectIndex = elementsToScan - 1;

    /*  Store the tokens in an array while using the index i to match subfolders names to node fields */
    int i = 0;
    char* tokens[MAX_TOKENS];
    tokens[i] = strtok(lineBuffer, "/"); // Returns the first token
    i++;
    while ((tokens[i] = strtok(NULL, "/")) != NULL) {
        // printf("Token %d: %s\n", i, tokens[i]);
        i++;
    }

    /* Validate year */
    if (isdigit(tokens[yearIndex][0])) {
        n->year = atoi(tokens[yearIndex]);
    } else {
        fprintf(stderr, "Invalid year: \"%s\". Exiting...\n", tokens[yearIndex]);        
        clearList(head);
        exit(EXIT_FAILURE);
    }
    
    snprintf(n->month, sizeof(n->month), "%s", tokens[monthIndex]);
    snprintf(n->client, sizeof(n->client), "%s", tokens[clientIndex]);
    snprintf(n->project, sizeof(n->project), "%s", tokens[projectIndex]);
    snprintf(n->path, sizeof(n->path), "%s", path);
    
    return n;
}


/***********************************************************************************
Function called from main, is the entry point to do the all the task. 
recursively scan subpaths and perform various tasks like add to the linked list,
escape single quote character which is necesary to later insert into the datababse
and skip scanning unnecesary subfolders. 
************************************************************************************/
void scanPath(const char *rootPath)
{
    char path[PATH_MAX];
    struct dirent *dirEntry;
    DIR *folder = opendir(rootPath);    
    // Unable to open directory stream
    if (folder == NULL) {
        perror("Unable to open directory stream");
        //exit(EXIT_FAILURE);      
        return;
    };  

    while ((dirEntry = readdir(folder)) != NULL)
    {
        if (dirEntry->d_type != DT_DIR) continue;

        if (strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0)
        {
            strcpy(path, rootPath);
            strcat(path, "/");
            strcat(path, dirEntry->d_name);
            
            /* Count slashes to stop scanning unnecesary subfolders */
            int slashCounter = 0, i = 0;
            while (path[i] != '\0') 
            {
                if (path[i] == '/')
                    slashCounter++;
                i++;                            
            }
                        
            /* Arrived to the last subfolder to scan, perform the insertion */
            if (slashCounter == elementsToScan)
            {                
                addPathToList(head, path);             
                continue; // Skip scanning the next subfolder
            }
            
            scanPath(path);
        }
    }

    closedir(folder);
}


void clearList(node* head)
{
    node* iter = head;
    node* prev = NULL;
    while (iter != NULL)
    {        
        prev = iter;
        iter = iter->next;
        free(prev);
    }
}


void printList(node* head)
{
    node* iter = head->next;
    int c = 1;
    while (iter != NULL)
    {
        printf("Node: %d\n", c);
        printNode(iter);        
        c++;
        iter = iter->next;
    }    
    printf("Number of nodes: %d\n", c);
}


void printNode(node* n)
{
    printf(" Project  %s\n", n->project);
    printf(" Client   %s\n", n->client);
    printf(" Path     %s\n", n->path);
    printf("\n");
}


void checkForValidArguments(int argc, char **argv)
{
    // Check for valid number of arguments
    if (argc < 4 || argc > 5)
    {
        printf("usage: %s /path/to/folder folder_level db_path [u]\n", argv[0]);
        printf("example create: %s /Volumes/MediaHdd/projects 7 /Users/user/Dcouments/projects.db\n", argv[0]);
        printf("example update: %s /Volumes/MediaHdd/projects 7 /Users/user/Dcouments/projects.db u\n", argv[0]);
        exit(EXIT_FAILURE);
    } 

    // Validate subfolder level to scan
    int res = atoi(argv[2]);
    if (res <= 0)
    {
        printf("Subfolder level must be a positive integer number.\n");
        exit(EXIT_FAILURE);
    }

    elementsToScan = res;

    // Check for a valid existing path 
    struct stat fileStats;
    if (stat(argv[1], &fileStats) == 0) 
    {
        printf("Checking if the path is a folder\n");
        if (!S_ISDIR(fileStats.st_mode)) {
            printf("Path is not a folder\n");
            exit(EXIT_FAILURE);
        }
    }
    else 
    {
        printf("Path doesn´t exists\n");
        exit(EXIT_FAILURE);
    }

    // Check database file
    char code = 0;
    if (stat(argv[3], &fileStats) == 0)
    {
        printf("Database already exists.\n");
        
        // Are we updating database?
        if (argc == 5 && argv[4][0] == 'u')
        {
            printf("Updating database\n");
            return;
        }
        printf("Press \'d\' to delete database or \'a\' to abort: ");
        scanf("%c", &code);
        code = tolower(code);

        switch (code)
        {
        case 'a':
        printf("Aborted.\n");
            exit(EXIT_SUCCESS);
            break; // Not necesary but ...         
        
        case 'd':
            /* Delete database */
            res = remove(argv[3]);
            if (res != 0)
            {
                printf("Couldn't remove file %s from disk", argv[3]);
                exit(EXIT_FAILURE);                
            }
            printf("Succesfully removed %s from disk\n", argv[3]);
            break;            
        
        default:
            printf("Invalid code.\n");
            exit(EXIT_SUCCESS);
            break;
        }
    }
}


/*  Fill the buffer of subfolders paths that will be scanned */
void foldersToScan(char** argv, int m, int n, char buf[m][n])
{
    struct dirent *dirEntry;
    DIR *folder = opendir(argv[1]);
    int i = 0;
    while ((dirEntry = readdir(folder)) != NULL)
    {
        if (dirEntry->d_type != DT_DIR) continue;        
        char* subfolder = dirEntry->d_name;
        if (atoi(subfolder) > 1900 && atoi(subfolder) < 9999 && i < m)
        {
            strcpy(buf[i], argv[1]);
            strcat(buf[i], "/");
            strcat(buf[i], subfolder);        
            i++;
        }
    }
     
    closedir(folder);
    
}


int getCurrentYear()
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    return tm.tm_year + 1900;
}
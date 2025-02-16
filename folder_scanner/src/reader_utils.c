#include "reader_utils.h"

#define MAX_TOKENS 32

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


node* new_node(const char *s)
{
    node *n = malloc(sizeof(node));
    if (n == NULL)
    {
        perror("Could not allocate memory for a new node");
        exit(EXIT_FAILURE);
    }
    n->next = NULL;
    strcpy(n->path, s);
    
    /* Are we creating an empty node as Head? */
    if (strcmp(s, "") == 0) {
        printf("Head created\n");
        return n;
    }

    /*  Need a buffer for strtok() function to parse the path */
    char *lineBuffer = strdup(s);  // Duplicate the s string for strtok()
    if (lineBuffer == NULL) {
        perror("Memory allocation failed for path buffer");
        free(n);
        exit(EXIT_FAILURE);
    }
    
    /*  Store the tokens in an array while using the index i to match subfolders names to node fields */
    /*  Probably could be better but it works */
    int yearIndex = elementsToScan - 4;
    int monthIndex = elementsToScan - 3;
    int clientIndex = elementsToScan - 2;
    int projectIndex = elementsToScan - 1;

    char* tokens[MAX_TOKENS] = {0}; // Array to save tokens
    const char* delimitor = "/";

    int i = 0;
    tokens[i] = strtok(lineBuffer, delimitor); // Returns the first token
    i++;
    while ((tokens[i] = strtok(NULL, delimitor)) != NULL) 
        i++;
    
    /* Validate year */
    if (isdigit(tokens[yearIndex][0])) { // tokens[yearIndex] && 
        n->year = atoi(tokens[yearIndex]);
    } else {
        fprintf(stderr, "Invalid year: \"%s\". Exiting...\n", tokens[yearIndex]);
        free(lineBuffer);
        clearList(head);
        exit(EXIT_FAILURE);
    }

    strcpy(n->month, tokens[monthIndex]);
    strcpy(n->client, tokens[clientIndex]);
    strcpy(n->project, tokens[projectIndex]);
    
    free(lineBuffer);
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
    // printf("addPathToList called\n");
    char path[PATH_MAX];
    struct dirent *dirEntry;
    DIR *folder = opendir(rootPath);
    printf("Scannig: %s\n", rootPath);
    // Unable to open directory stream
    if (folder == NULL) {
        perror("Unable to open directory stream\n");
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
                /* Deal with sigle quotes character before insert into database */                
                char c = '\'';
                char* singleQuote = strchr(path,c);
                if (singleQuote != NULL)
                {
                    int pos = singleQuote - path;
                    escapeSingleQuote(path, singleQuote, pos);
                }                
                addPathToList(head, path);
                printf("Added: %s\n", path);
                continue; // Skip scanning the next subfolder
            }
            printf("\n");
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

/*************************************************************************
  Sqlite3 escape single quote in a diferent way than C. This is necesary 
  to inert strings like "D'Omnion" into text filed database.
*************************************************************************/
void escapeSingleQuote(char* p, const char* r, int idx) // 
{
    char lineBuffer[PATH_MAX];
    strncpy(lineBuffer, p, idx);
    strcat(lineBuffer, "\'");
    strcat(lineBuffer, r);
    strcat(lineBuffer, "\0");
    strcpy(p,lineBuffer);
}


bool checkForValidArguments(int argc, char **argv)
{
    // Check for valid number of arguments
    if (argc != 4) 
    {
        printf("usage: %s /path/to/folder folder_level db_path \n", argv[0]);
        printf("example: %s /Volumes/MediaHdd/projects 7 /Users/user/Dcouments/projects.db\n", argv[0]);
        return false;
    }


    // Check for subfolder level to scan
    int res = atoi(argv[2]);
    if (res <= 0)
    {
        printf("level of subfolder must be a positive integer number.\n");
        return false;
    }

    elementsToScan = res;

    // Check for a valid existing path 
    struct stat fileStats;
    if (stat(argv[1], &fileStats) == 0) 
    {
        printf("Checking if the path is a folder\n");
        if (!S_ISDIR(fileStats.st_mode)) {
            printf("Path is not a folder\n");
            return  -2;
        }
    }
    else 
    {
        printf("Path doesn´t exists\n");
        return -3;
    }

    // Check database file
    char code = 0;
    if (stat(argv[3], &fileStats) == 0)
    {
        printf("Database already exists.\n");
        printf("Press \'u\' to update, \'d\' to delete database or \'a\' to abort: ");
        scanf("%c", &code);
        code = tolower(code);

        switch (code)
        {
        case 'a':
        printf("Aborted.\n");
            exit(EXIT_SUCCESS);
            break; // Not necesary but ... 
        
        case 'u':
            /* Update database */
            printf("Updating database ...\n");
            /* TODO */
            break;
        
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
    return true;
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

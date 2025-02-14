/***************************************************************************************
  reader_utils: This module provide a linked list data structure to store the projects 
  folder paths as nodes that later, will be inserted into a table in a sqlite3 database.

  Created by Américo Cot on 02/11/22.
****************************************************************************************/


#ifndef _READER_UTILS_H_
#define _READER_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <dirent.h>
#include <ctype.h>

/***************************************************************************************
    Data Structure  - Linked list 
    Path ex: /Volumes/MediaHD/backups/year/month/client/project/files&subfolders
****************************************************************************************/
typedef struct node
{
    int year;
    char month[32];
    char client[NAME_MAX];
    char project[NAME_MAX];
    char path[PATH_MAX];
    struct node *next;
} node;


/* Functions declarations */
node* new_node(const char* s);
void printNode(node* n);

void scanPath(const char* rootPath);
void addPathToList(node* head, const char* path);
void printList(node* head);
void clearList(node* head);

bool checkForValidArguments(int argc, char** argv);
void escapeSingleQuote(char* p, const char* r, int idx);

void foldersToScan(char** argv, int m, int n, char buf[m][n]);

#endif

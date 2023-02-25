# CS50 Final Project

## Video demo:  <https://youtu.be/R-YTGx4Nr9c>

## Description: This project is a solution for my daily work.

The solution scans a folder where we store our projects, organised by year, month, client and project name, and then creates a database where we can search for a project and access it more quickly.

The solution has two parts:

1. A C command line utility called "folder_scanner" that scans a root folder provided by the user and creates a data structure in the form of a single linked list based on the folder structure, after which the utility will store this data in a sqlite3 database.

2. A MacOS application developed in Swift that provides a graphical user interface to read and interact with the data.

The data is a full path representing a project as a node in the linked list, and the path is composed of subfolders representing members of the node.

For example:

~~~c
Path: /Volumes/MacSSD/backups/year/month/client/project/subfolders+files

typedef struct node
{
    char year[5];
    char month[32];
    char client[NAME_MAX];
    char project[NAME_MAX];
    char path[PATH_MAX];
    struct node *next;
} node;
~~~

Here the root folder is "/Volumes/MacSSD/backups/" and the full path is parsed into a node and added to the linked list.

The linked list is stored in a database.
The database is accessed by users with the MacOS app which has a search field to search for a project or client in the database and a table view to display the results of the search, once the result is displayed in the table view the user can double click on a row to open that project folder in the Finder.

For the MacOS app, I added the package [SQlite.swift](https://github.com/stephencelis/SQLite.swift.git) by Stephen Celis, which provides a "type-safe, Swift-language layer over SQLite3" to read the database from the swift app.



## Usage

~~~bash
cmd:
./folder_scanner '/path/to/rootfolder' folder_level '/path/to/database'

ex:
./folder_scanner '/Volumes/hdd/users/user/projects' 6 '~/Dcouments/projects.db'
~~~

In this particular case, folder_level is 6, because the project name is the sixth element in the path:

~~~~bash
/Volumes/NAS_AUDIO/2022/DECEMBER/CLIENT/PROJECT_NAME/'
    1        2      3       4      5       6
~~~~

If the database doesn't exist, it will be created. If the database exists, you'll be asked to update the database, delete the database or cancel the scan.

Once the database has been created and the data inserted, a user can search for a project using the MacOS 'Projects' application. The application will search the database and return a list of records matching the search text in the Project or Client column.
When the user double-clicks on a row representing a project, the project folder is opened in the Finder.

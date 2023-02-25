/***************************************************************************************
  db_utils: This module has only two tasks, create a database and insert a node into the
  database.

  Created by Américo Cot on 02/11/22.
****************************************************************************************/

#ifndef _DB_UTILS_H_
#define _DB_UTILS_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "reader_utils.h"

/* Database related functions */
int createDataBase(const char* DBName);
int insertData(node* head, const char* DBName);

#endif

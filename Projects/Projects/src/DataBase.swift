//
//  DataBase.swift
//  Projects
//
//  Created by Américo Cot on 27/11/22.
//
//  Database class provide funcionality to connect to a Sqlite3 database,
//  search in the database and return an array of results, print
//  database info and present sqlite errors through MainViewController using
//  using 'DatabaseControllerDelegate' protocol.
//  This class inplement Sqlite.swift package by Stephen Celis.
//  Git repo: https://github.com/stephencelis/SQLite.swift.git
//


import Foundation
import SQLite

/*  Protocol to pass Sqlite errors to MainViewController */
protocol DatabaseControllerDelegate : NSObjectProtocol {
    func showMessagge(error: NSError)
    func showAlert(text: String)
}

/*  Database */
class DataBase {
    var db: Connection?
    let schema: SchemaReader!
    var dbTableName: String!
    var dbTableColumnsCount: Int!
    var dbTableColumnsNames: [String]!
    var isConected: Bool = false
    
    /*  Variables that serve as TableView Datasource */
    var searchResult: [Row]!
    var rows: Int!
    
    /*  Variables needed to interact with the database */
    var projects:Table!
    var id:SQLite.Expression<Int64>!
    var project:SQLite.Expression<String>!
    var client:SQLite.Expression<String>!
    var year:SQLite.Expression<Int>!
    var month:SQLite.Expression<String>!
    var path:SQLite.Expression<String>!
    
    /*  Variable to used to pass Sqlite errors to MainViewController */
    var delegateDB: DatabaseControllerDelegate?
    

    init(path: String) {
        do {            
            self.db = try Connection(path, readonly: false)
            self.isConected = true
        }   catch let error as NSError {
            print(error.localizedDescription)
        }
        
        self.searchResult = []
        self.rows = 0
        
        self.projects = Table("projects")
        self.id = Expression<Int64>("id")
        self.project = Expression<String>("project")
        self.client = Expression<String>("client")
        self.year = Expression<Int>("year")
        self.month = Expression<String>("month")
        self.path = Expression<String>("path")
        
        self.schema = db?.schema
        self.dbTableColumnsNames = []
        
        /* Test database format */
        if isConected {
            do {
                let _ = try db?.schema.columnDefinitions(table: "projects")
            } catch let error as NSError {
                print(error.localizedDescription)
                /* May be conected but not to the correct one */
                isConected = false
            }
        }
    }
    
    
    /*  Function called from MVC to query the database, the result of the query is stored in searchResult array */
    func searchInDB(text: String) {
        if text.isEmpty {
            searchResult.removeAll()
            self.rows = 0
            return
        }
        
        if isConected {
            searchResult.removeAll()
            var stringToSearch = text
            
            stringToSearch = "%" + text + "%"
            let filtered = projects.filter((project.like(stringToSearch, escape: "\'")) ||
                                           (client.like(stringToSearch))).order(year.desc, month)
                
            do {
                searchResult = Array(try (db?.prepare(filtered))!)   // SELECT * FROM "projects" WHERE ...
                self.rows = searchResult.count
            } catch let error as NSError {
                delegateDB?.showMessagge(error: error)
            }
        } else {
            delegateDB?.showAlert(text: "No connection to database\nor invalid database.\nSelect a database")
            print("No DATABASE")
        }
    }
    
    
    /* Insert path dragged into projects list interface */
    func insertPathInDB(pathToInsert: String) {
        if isConected {
            var insertable: Bool = false
            var monthToIns = ""
            var clientToIns = ""
            var projectToIns = ""
            var pathToInsR = ""
            var yearToIns = 0
            let tokens = pathToInsert.components(separatedBy: "/")
            var pathYear: Int!
            for (i,token) in tokens.enumerated() {
                pathYear = Int(token) ?? 0
                if (pathYear > 1900 && pathYear <= 9999) {
                    insertable = true
                    yearToIns = pathYear
                    monthToIns = tokens[i+1].replacingOccurrences(of: "%20", with: " ")
                    clientToIns = tokens[i+2].replacingOccurrences(of: "%20", with: " ")
                    projectToIns = tokens[i+3].replacingOccurrences(of: "%20", with: " ")
                    pathToInsR = pathToInsert.replacingOccurrences(of: "%20", with: " ")
                    break
                }
            }
            
            if (insertable) {
                print("Inserting: \(projectToIns), \(clientToIns), \(monthToIns), \(pathToInsR)")
                /* Insert record ito Database */
                do {
                    try db?.run(projects.insert(month<-monthToIns,
                                            client<-clientToIns,
                                            project<-projectToIns,
                                            year<-yearToIns,
                                            path<-pathToInsR))
                                
                } catch let error as NSError {
                    delegateDB?.showMessagge(error: error)
                }
            }
        } else {
            delegateDB?.showAlert(text: "Not conected to a database.\nSelect a valid database")
        }
        
    }
    
    
    /*  Info about the Database */
    func printDBInfo() {
        do {
            let table = try schema.objectDefinitions(type: .table)
            dbTableName = table[0].tableName
            
            print("Table Name: \(String(describing: dbTableName)), type: \(table[0].type )")
            let columns = try schema.columnDefinitions(table: dbTableName)
            dbTableColumnsCount = columns.count
            for c in columns {
                dbTableColumnsNames.append(c.name)
                
                print("Column Name: \(c.name), type: \(c.type)")
            }
        } catch let error as NSError {
            delegateDB?.showMessagge(error: error)
            print(error.localizedDescription)
        }        
    }
    
}

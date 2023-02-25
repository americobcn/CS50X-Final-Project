//
//  MainViewController.swift
//  Projects
//
//  Created by Américo Cot on 26/11/22.
//
//  Following the Model View Controller paradigm, this class present persistent data stored in a Sqlite database
//  to the user through a Table View. This class implement the 'NSTableViewDelegate' and 'NSTableViewDataSource' methods
//  to provide data from the Database object to be displayed in the table view.
//

import Cocoa

class MainViewController: NSViewController, NSTableViewDataSource, NSTableViewDelegate, DatabaseControllerDelegate, ProjectsViewControllerDelegate {
    
    /* Variables that connect the MVC to the user interface file (.xib) */
    @IBOutlet weak var searchField: NSSearchField!
    @IBOutlet weak var tableView: NSTableView!
    @IBOutlet var mainWindow: NSWindow!
    @IBOutlet weak var projectsView: ProjectsView!
    
    /*  Variables that sotre yhe connection to a Database located at specified path */
    var dataBase: DataBase!
    var path: String!
    
    /*  Constants used by the table view delegate method to identify which cell view on a table column      */
    /*  need to be filled when the delegate method of the tbale view is called                              */
    let projectCell =  NSUserInterfaceItemIdentifier(rawValue: "projectCellID")
    let clientCell = NSUserInterfaceItemIdentifier(rawValue: "clientCellID")
    let monthCell = NSUserInterfaceItemIdentifier(rawValue: "monthCellID")
    let yearCell = NSUserInterfaceItemIdentifier(rawValue: "yearCellID")
    let pathCell = NSUserInterfaceItemIdentifier(rawValue: "yearCellID")
    let idCell = NSUserInterfaceItemIdentifier(rawValue: "yearCellID")
    
    let projectCol =  NSUserInterfaceItemIdentifier(rawValue: "projectColumnID")
    let clientCol = NSUserInterfaceItemIdentifier(rawValue: "clientColumnID")
    let monthCol = NSUserInterfaceItemIdentifier(rawValue: "monthColumnID")
    let yearCol = NSUserInterfaceItemIdentifier(rawValue: "yearColumnID")
    let pathCol = NSUserInterfaceItemIdentifier(rawValue: "yearColumnID")
    let idCol = NSUserInterfaceItemIdentifier(rawValue: "yearColumnID")
    
    
    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        path = UserDefaults.standard.string(forKey: "DBPath") ?? String()
        
        /*  Check if file exists at 'path' */
        let fileManager = FileManager.default
        if (fileManager.fileExists(atPath: path)) {
            self.dataBase = DataBase(path: path)
            /*  Set the mvc to be the delegate of the database to recive errors messages from the sqlite api. */
            dataBase.delegateDB = self
        } else {
            print("No Database")
            self.selectDB()
        }
    }
    
    
    override func viewDidLoad() {
        super.viewDidLoad()
        /* Set the delegates to implement the table view */
        tableView.target = self
        tableView.dataSource = self
        tableView.delegate = self
        
        /* Change the defalut app name to 'app name: database_name' to provide visual info about the current database been used.     */
        self.view.window?.title = "Projects: " + URL(fileURLWithPath: path).lastPathComponent
        
    }
    
    override func awakeFromNib() {
        super.awakeFromNib()
        projectsView.projectsViewDelegate = self
        if !self.dataBase.isConected {
            self.showAlert(text: "Not conected to a database.\nSelect a valid database")
        }
    }
    
    
    /*  This action method open project folder in the finder when user double click in a project row */
    @IBAction func doubleClicked(_ sender: AnyObject) {
        let data = try! dataBase.searchResult[sender.clickedRow].get(dataBase.path) + "/"
        let url = URL(fileURLWithPath: data)
        print(url)
        NSWorkspace.shared.open(url)
    }
    
    
    /* Send the text to search to the databse object and refresh the view with the new reults */
    @IBAction func searchDB(_ sender: Any) {
        dataBase?.searchInDB(text: searchField.stringValue)
        tableView.reloadData()
    }
    
    
    /* TableView delegate and datasource methods */
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        /* databse.rows store the count of items in the array 'seacrhResuts' in the object database */
        return dataBase.rows!
    }
    
    func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
        
        var text: String = ""
        var cellIdentifier = NSUserInterfaceItemIdentifier(rawValue: "")
        guard let colIdentifier = tableColumn?.identifier else { return nil }
        
        /* Select the value to display in a cell view at specified column switching on 'colIdentifier' for the current row */
        switch colIdentifier {
            case projectCol:
                cellIdentifier = projectCell
                text = try! dataBase.searchResult[row].get(dataBase.project)
            case clientCol:
                cellIdentifier = clientCell
                text = try! dataBase.searchResult[row].get(dataBase.client)
            case monthCol:
                cellIdentifier = monthCell
                text = try! dataBase.searchResult[row].get(dataBase.month)
                text.removeSubrange(text.startIndex..<text.index(text.startIndex, offsetBy: 2))
            case yearCol:
                cellIdentifier = yearCell
                text = try! String(dataBase.searchResult[row].get(dataBase.year))
            case pathCol:
                cellIdentifier = pathCell
                text = try! String(dataBase.searchResult[row].get(dataBase.path))
            case idCol:
                cellIdentifier = idCell
                text = try! String(dataBase.searchResult[row].get(dataBase.id))
            default:
                text = "NaD"
                break
        }
        
        /*  Initialize a cell view with current identifier been evaluated */
        guard let cell = self.tableView.makeView(withIdentifier: cellIdentifier, owner: self) as? NSTableCellView else {
            return nil
        }
        /*  Set the value to be displayed in the cell view   */
        cell.textField?.stringValue = text
        return cell
    }
    
    /* Action for the Open Database menu item */
    @IBAction func openDatabase(_ sender: Any) {
        self.selectDB()
        self.tableView.reloadData()
        self.mainWindow.title = "Projects: " + path
    }
    
    /*  func to select and open a database */
    func selectDB() {
        /* Init and setup of the finder file selector */
        let panel = NSOpenPanel()
        panel.title = "Choose a Database"
        panel.message = "Select a valid sqlite3 Database"
        panel.showsResizeIndicator = true
        panel.showsHiddenFiles = false
        panel.canChooseFiles = true
        panel.canChooseDirectories = false
        panel.allowsMultipleSelection = false
        
        /* Show the panel and process the result   */
        if (panel.runModal() == NSApplication.ModalResponse.OK) {
            let result = panel.url
            if (result != nil) {
                self.path = result!.path
                /* Store the path of the selected database */
                UserDefaults.standard.set(path, forKey: "DBPath")
                /* Open the database */
                self.dataBase = DataBase(path: path)
                /*  Set the mvc to be the delegate of the database to recive errors messages from the sqlite api */
                dataBase.delegateDB = self
            }
        } else {
            /* Present a message and quit app if the action is canceled and no database is stored in the user defaults */
            let alert = NSAlert()
            alert.alertStyle = .informational
            alert.messageText = "No database selected.\nQuiting ..."
            alert.runModal()
            exit(-1)
        }
        
    }
    
    
    /* Database delegate method  */
    /** Function called in the delegate variable of the database object to present a warnig error ocurred in the database  **/
    func showMessagge(error: NSError) {
        print("MVC: \(error)")
        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.messageText = error.localizedDescription + "\nSelect a valid Database"
        alert.runModal()
        self.selectDB()
    }
    
    func showAlert(text: String) {
        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.messageText = text
        alert.runModal()
        self.selectDB()
    }
    
    
    /* ProjectsView delegate method  */
    /** Function called in the delegate variable of the projectsView to insert dragged url into the database **/
    func processURL(url: URL) {
        let pathStr = url.absoluteString
        let startIdx = pathStr.index(pathStr.startIndex, offsetBy: 7)
        let pathToInsert = String(pathStr[startIdx..<pathStr.endIndex].dropLast())
        dataBase.insertPathInDB(pathToInsert: pathToInsert)
    }
    
}

//
//  AppDelegate.swift
//  Projects
//
//  Created by Américo Cot on 26/11/22.
//

import Cocoa

@main
class AppDelegate: NSObject, NSApplicationDelegate {

    @IBOutlet var window: NSWindow!


    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Setup an entry to store the database location path
        let defaults = UserDefaults.standard
        let dict = ["DBPath":""]
        defaults.register(defaults: dict)
        let dbName = defaults.object(forKey: "DBPath") as? String ?? ""
        // Use the path as a part of the title of main window app
        self.window.title = "Projects: " + dbName
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
    }

    func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool {
        return true
    }


}


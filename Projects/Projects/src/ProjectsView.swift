//
//  ProjectsView.swift
//  Projects:
//  Add drag and drop funcionality to give the user the oportunity to insert a new project record just dragging the
//  project path onto the view app.
//
//  Created by Américo Cot Toloza on 28/12/22.
//

import Cocoa

/* Protocol to be implemented by MainViewController to handle the drag and drop operation */
protocol ProjectsViewControllerDelegate : NSObjectProtocol {
    func processURL(url: URL)
}

class ProjectsView: NSView {
    
    var projectsViewDelegate: ProjectsViewControllerDelegate?
    let supportedTypes: [NSPasteboard.PasteboardType] = [ .URL, .fileURL]
    
    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)
        
    }
    
    override func awakeFromNib() {
        super.awakeFromNib()
        self.registerForDraggedTypes(supportedTypes)
        
    }
    
    //    MARK: Drag and Drop
        override func prepareForDragOperation(_ sender: NSDraggingInfo) -> Bool {
            return true
        }
        
        override func draggingEntered(_ sender: NSDraggingInfo) -> NSDragOperation {
            let canReadPasteboardObjects = sender.draggingPasteboard.canReadObject(forClasses: [NSURL.self], options: nil)  // supportedTypes
            
            /* If dragged object are of the expected type, highlight the border view */
            if canReadPasteboardObjects {
                highlight()
                   return .copy
               }
            
            return NSDragOperation()
        }
        
        
        override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {
            sender.draggingDestinationWindow?.orderFrontRegardless()
            guard let pasteboardObjects = sender.draggingPasteboard.readObjects(forClasses: [NSURL.self], options: nil), pasteboardObjects.count > 0 else {
                return false
            }
            
            /* Get the dragged objects */
            pasteboardObjects.forEach { (object) in
                if let url = object as? NSURL {
                    /* Send the urls objects to the delgate to be processed */
                    projectsViewDelegate?.processURL(url: url as URL)
                }
                
                unhighlight()
            }
            
            return true
        }
            
        /* Highlight view borders on entering the view area */
        func highlight() {
            self.layer?.borderColor = NSColor.controlAccentColor.cgColor
            self.layer?.borderWidth = 0.8
        }
        
        /* Unhighlight when droped or exiting view area */
        func unhighlight() {
            self.layer?.borderColor = NSColor.black.cgColor
            self.layer?.borderWidth = 0.5
        }
}

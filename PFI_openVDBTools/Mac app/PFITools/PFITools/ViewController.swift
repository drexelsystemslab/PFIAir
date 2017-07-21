//
//  ViewController.swift
//  PFITools
//
//  Created by Hanjie Liu on 7/20/17.
//  Copyright © 2017 Hanjie Liu. All rights reserved.
//

import Cocoa

class ViewController: NSViewController {

    @IBOutlet weak var textfield: NSTextField!
    var fileURL: URL?
    
    let manager = NetworkManger(ip: "127.0.0.1", port: "8000", suffix: "/api/search/")
    
    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
    }
    
    @IBAction func buttonPressed(_ sender: Any) {
        let dialog = NSOpenPanel();
        
        dialog.title = "Choose a .stl file";
        dialog.showsResizeIndicator = true;
        dialog.showsHiddenFiles = false;
        dialog.canChooseDirectories = false;
        dialog.canCreateDirectories = true;
        dialog.allowsMultipleSelection = false;
        dialog.allowedFileTypes = ["stl"];
        
        if (dialog.runModal() == NSModalResponseOK) {
            if let result = dialog.url {
                fileURL = result
                textfield.stringValue = result.path
            }
        }
    }
    
    @IBAction func submit(_ sender: Any) {
        if let fileURL = fileURL {
            manager.search(filepath: fileURL)
        }
    }

}


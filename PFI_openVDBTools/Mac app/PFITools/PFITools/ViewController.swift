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
    @IBOutlet weak var filePathLabel: NSTextField!
    
    var fileURL: URL?
    //var models: [Model]?
    
    let manager = NetworkManger(ip: "127.0.0.1", port: "8000")
    let fmanager = FileIOManager(path: "/Users/Hollis/Developer/PFIAir/PFIAir/PFI_openVDBTools/Mac app/PFITools/DerivedData/sandbox")
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        filePathLabel.stringValue = fmanager.filePath
    }
    
    @IBAction func selectSavePath(_ sender: Any) {
        let dialog = NSOpenPanel();
        
        dialog.title = "Choose a directory to save model files"
        dialog.showsResizeIndicator = true
        dialog.showsHiddenFiles = false
        dialog.canChooseFiles = false
        dialog.canChooseDirectories = true
        dialog.canCreateDirectories = true
        dialog.allowsMultipleSelection = false
        dialog.allowedFileTypes = ["stl"]
        
        if (dialog.runModal() == NSModalResponseOK) {
            if let result = dialog.url {
                filePathLabel.stringValue = result.path
                fmanager.filePath = result.path
            }
        }
        
    }
    
    @IBAction func buttonPressed(_ sender: Any) {
        let dialog = NSOpenPanel();
        
        dialog.title = "Choose a .stl file"
        dialog.showsResizeIndicator = true
        dialog.showsHiddenFiles = false
        dialog.canChooseDirectories = false
        dialog.canCreateDirectories = true
        dialog.allowsMultipleSelection = false
        dialog.allowedFileTypes = ["stl"]
        
        if (dialog.runModal() == NSModalResponseOK) {
            if let result = dialog.url {
                fileURL = result
                textfield.stringValue = result.path
            }
        }
    }
    
    @IBAction func submit(_ sender: Any) {
        if let fileURL = fileURL {
            manager.search(filepath: fileURL) { mod in
                if let models = mod {
                    for m in models {
                        self.downloadModel(mod: m)
                        print(m)
                    }
                }
            }
        }
    }
    
    private func downloadModel(mod: Model) {
        manager.download(id: mod.id) { data in
            self.fmanager.saveFile(data: data, name: mod.name, extensionName: "stl")
        }
    }

}


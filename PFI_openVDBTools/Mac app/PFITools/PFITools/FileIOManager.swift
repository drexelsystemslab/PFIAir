//
//  FileManager.swift
//  PFITools
//
//  Created by Hanjie Liu on 7/21/17.
//  Copyright © 2017 Hanjie Liu. All rights reserved.
//

import Foundation

class FileIOManager {
    var filePath = ""
    
    func saveFile(data:Data, name: String, extensionName: String) {
        let fileURL = URL(fileURLWithPath: "\(filePath)/\(name).\(extensionName)")
        
        do {
            try data.write(to: fileURL, options: .atomic)
        } catch {
            print(error)
        }
    }
    
    init(path: String) {
        filePath = path
    }
}

//
//  NetworkManager.swift
//  PFITools
//
//  Created by Hanjie Liu on 7/20/17.
//  Copyright © 2017 Hanjie Liu. All rights reserved.
//

import Foundation
import Alamofire

class NetworkManger {
    private var ip = ""
    private var port = ""
    private var suffix = ""
    
    func search(filepath: String) {
        let parameters: Parameters = ["file": filepath]
        
        let serverPath = "http://\(ip):\(port)\(suffix)"
        
        let fileURL = URL(fileURLWithPath: filepath, isDirectory: true)
        print(fileURL)
        
        Alamofire.upload(fileURL, to: serverPath).responseJSON { response in
            debugPrint(response)
        }
        
//        Alamofire.request(serverPath, method: .post, parameters: parameters).responseJSON { response in
//            debugPrint(response)
//            if let json = response.result.value {
//                print("JSON: \(json)")
//            }
//        }
    }
    
    init(ip: String, port: String,suffix: String) {
        self.ip = ip
        self.port = port
        self.suffix = suffix
    }
}

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
    
    func search(filepath: URL) {        
        let serverPath = "http://\(ip):\(port)\(suffix)"
        
        Alamofire.upload(
            multipartFormData: { multipartFormData in
            multipartFormData.append(filepath, withName: "file")
        },
            to: serverPath,
            encodingCompletion: { encodingResult in
                switch encodingResult {
                case .success(let upload, _, _):
                    upload.responseJSON { response in
                        debugPrint(response)
                    }
                case .failure(let encodingError):
                    print(encodingError)
                }
        })

    }
    
    init(ip: String, port: String, suffix: String) {
        self.ip = ip
        self.port = port
        self.suffix = suffix
    }
}

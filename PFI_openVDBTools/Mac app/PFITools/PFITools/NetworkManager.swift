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
                        print(self.parseJSON(result: response.data!))
                    }
                case .failure(let encodingError):
                    print(encodingError)
                }
        })
    }
    
    
    private func parseJSON(result: Data) -> [Model]?{
        do {
            let returnedJSON = try JSONSerialization.jsonObject(with: result, options: [])
            
            if let dict = returnedJSON as? [String : Any] {
                guard let models = dict["models"] as? [Any] else {return nil}
                
                var mods = [Model]()
                
                for mod in models {
                    let model = mod as! [String : Any]
                    
                    let newModel = Model(name: model["name"] as! String, distance: model["distance"] as! Double, downloadLocation: model["location"] as! String, previewLocation: model["preview"] as! String, id: model["id"] as! Int)
                    
                    mods.append(newModel)
                }
                
                return mods
            }
        } catch {
            print(error)
        }
        
        return nil
    }
    
    init(ip: String, port: String, suffix: String) {
        self.ip = ip
        self.port = port
        self.suffix = suffix
    }
}

//
//  NetworkManager.swift
//  PFITools
//
//  Created by Hanjie Liu on 7/20/17.
//  Copyright © 2017 Hanjie Liu. All rights reserved.
//

import Foundation
import Alamofire

struct Suffix {
    static let search = "/api/search/"
    static let download = "/api/download/"
}

class NetworkManger {
    private var ip = ""
    private var port = ""
    
    /// search server for similar 3D models based on a given 3D model
    func search(filepath: URL, completion: @escaping (_ mod: [Model]?) -> ()) {
        let serverPath = "http://\(ip):\(port)\(Suffix.search)"
        
        Alamofire.upload(
            multipartFormData: { multipartFormData in
            multipartFormData.append(filepath, withName: "file")
        },
            to: serverPath,
            encodingCompletion: { encodingResult in
                switch encodingResult {
                case .success(let upload, _, _):
                    upload.responseJSON { response in
                        let modelResults = self.parseJSON(result: response.data!)
                        completion(modelResults)
                    }
                case .failure(let encodingError):
                    print(encodingError)
                }
        })
    }
    
    /// request .stl files on server based on ID
    func download(id: Int, completion: @escaping (_ data: Data) -> ()) {
        let serverPath = "http://\(ip):\(port)\(Suffix.download)\(id)"

        Alamofire.request(serverPath, method: .post).responseData { response in
            if let data = response.data {
                completion(data)
            }
        }
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
    
    init(ip: String, port: String) {
        self.ip = ip
        self.port = port
    }
}

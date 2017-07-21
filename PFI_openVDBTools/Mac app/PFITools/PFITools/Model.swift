//
//  Model.swift
//  PFITools
//
//  Created by Hanjie Liu on 7/21/17.
//  Copyright © 2017 Hanjie Liu. All rights reserved.
//

import Foundation

struct Model {
    var name = ""
    var distance = 0.0
    var downloadLocation = ""
    var previewLocation = ""
    var id = 0

    init(name: String, distance: Double, downloadLocation: String, previewLocation: String, id: Int) {
        self.name = name
        self.distance = distance
        self.downloadLocation = downloadLocation
        self.previewLocation = previewLocation
        self.id = id
    }
}

//
//  PreviewView.swift
//  Depth Mesh
//
//  Created by Hanjie Liu on 3/8/18.
//  Copyright © 2018 Drexel. All rights reserved.
//

import UIKit
import AVFoundation

class VideoPreviewView: UIView {
    var videoPreviewLayer: AVCaptureVideoPreviewLayer {
        return layer as! AVCaptureVideoPreviewLayer
    }
    var session: AVCaptureSession? {
        get { return videoPreviewLayer.session }
        set { videoPreviewLayer.session = newValue }
    }
    override class var layerClass: AnyClass {
        return AVCaptureVideoPreviewLayer.self
    }
    private var orientationMap: [UIDeviceOrientation : AVCaptureVideoOrientation] = [
        .portrait           : .portrait,
        .portraitUpsideDown : .portraitUpsideDown,
        .landscapeLeft      : .landscapeRight,
        .landscapeRight     : .landscapeLeft,
        ]
    func updateVideoOrientationForDeviceOrientation() {
        if let videoPreviewLayerConnection = videoPreviewLayer.connection {
            let deviceOrientation = UIDevice.current.orientation
            guard let newVideoOrientation = orientationMap[deviceOrientation],
                deviceOrientation.isPortrait || deviceOrientation.isLandscape
                else { return }
            videoPreviewLayerConnection.videoOrientation = newVideoOrientation
        }
    }
}

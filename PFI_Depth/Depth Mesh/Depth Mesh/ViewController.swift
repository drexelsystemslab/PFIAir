//
//  ViewController.swift
//  Depth Mesh
//
//  Created by Hanjie Liu on 3/8/18.
//  Copyright © 2018 Drexel. All rights reserved.
//
// Some portion of this file is refernced from raywenderlich.com
// at https://www.raywenderlich.com/185885/video-depth-maps-tutorial-for-ios-getting-started

import UIKit
import AVFoundation
import Photos
import MobileCoreServices


enum PreviewMode: Int {
    case original
    case depth
}

class ViewController: UIViewController {

    @IBOutlet weak var previewView: VideoPreviewView!
    @IBOutlet weak var previewModeControl: UISegmentedControl!
    private let photoOutput = AVCapturePhotoOutput()

    private let sessionQueue = DispatchQueue(label: "session queue", attributes: [], autoreleaseFrequency: .workItem)
    private let processingQueue = DispatchQueue(label: "photo processing queue", attributes: [], autoreleaseFrequency: .workItem)
    
    var previewMode = PreviewMode.original
    let dataOutputQueue = DispatchQueue(label: "video data queue",
                                        qos: .userInitiated,
                                        attributes: [],
                                        autoreleaseFrequency: .workItem)
    
    var background: CIImage?
    var depthMap: CIImage?
    var scale: CGFloat = 0.0

    @IBOutlet weak var previewImage: UIImageView!
    
    let session = AVCaptureSession()
    var captureDepthOutput: AVCaptureDepthDataOutput?
    var isCaptureSessionConfigured = false
    
    @IBAction func modeChange(_ sender: UISegmentedControl) {
        previewMode = PreviewMode(rawValue: previewModeControl.selectedSegmentIndex) ?? .original
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        
        previewMode = PreviewMode(rawValue: previewModeControl.selectedSegmentIndex) ?? .original

        
        configureCaptureSession()
        
        session.startRunning()
        
    }
    

    @IBAction func shutterPressed(_ sender: Any) {
        sessionQueue.async {
            let photoSettings = AVCapturePhotoSettings(format: [kCVPixelBufferPixelFormatTypeKey as String: Int(kCVPixelFormatType_32BGRA)])
            //if self.photoOutput.isDepthDataDeliverySupported {
                photoSettings.isDepthDataDeliveryEnabled = true
                photoSettings.embedsDepthDataInPhoto = false
            //}


            self.photoOutput.capturePhoto(with: photoSettings, delegate: self)
        }
    }
    
    
    func checkPhotoLibraryAuthorization(_ completionHandler: @escaping ((_ authorized: Bool) -> Void)) {
        switch PHPhotoLibrary.authorizationStatus() {
        case .authorized:
            // The user has previously granted access to the photo library.
            completionHandler(true)
            
        case .notDetermined:
            // The user has not yet been presented with the option to grant photo library access so request access.
            PHPhotoLibrary.requestAuthorization({ status in
                completionHandler((status == .authorized))
            })
            
        case .denied:
            // The user has previously denied access.
            completionHandler(false)
            
        case .restricted:
            // The user doesn't have the authority to request access e.g. parental restriction.
            completionHandler(false)
        }
    }

    func checkCameraAuthorization(_ completionHandler: @escaping ((_ authorized: Bool) -> Void)) {
        switch AVCaptureDevice.authorizationStatus(for: AVMediaType.video) {
        case .authorized:
            //The user has previously granted access to the camera.
            completionHandler(true)
            
        case .notDetermined:
            // The user has not yet been presented with the option to grant video access so request access.
            AVCaptureDevice.requestAccess(for: AVMediaType.video, completionHandler: { success in
                completionHandler(success)
            })
            
        case .denied:
            // The user has previously denied access.
            completionHandler(false)
            
        case .restricted:
            // The user doesn't have the authority to request access e.g. parental restriction.
            completionHandler(false)
        }
    }

}

// MARK: - Helper Methods

extension ViewController {
    private class func jpegData(withPixelBuffer pixelBuffer: CVPixelBuffer, attachments: CFDictionary?) -> Data? {
        let ciContext = CIContext()
        let renderedCIImage = CIImage(cvImageBuffer: pixelBuffer)
        guard let renderedCGImage = ciContext.createCGImage(renderedCIImage, from: renderedCIImage.extent) else {
            print("Failed to create CGImage")
            return nil
        }
        
        guard let data = CFDataCreateMutable(kCFAllocatorDefault, 0) else {
            print("Create CFData error!")
            return nil
        }
        
        guard let cgImageDestination = CGImageDestinationCreateWithData(data, kUTTypeJPEG, 1, nil) else {
            print("Create CGImageDestination error!")
            return nil
        }
        
        CGImageDestinationAddImage(cgImageDestination, renderedCGImage, attachments)
        if CGImageDestinationFinalize(cgImageDestination) {
            return data as Data
        }
        print("Finalizing CGImageDestination error!")
        return nil
    }
    
    func configureCaptureSession() {
        
        guard let camera = AVCaptureDevice.default(.builtInDualCamera,
                                                   for: .video,
                                                   position: .unspecified) else {
                                                    
                                                    fatalError("No depth video camera available")
        }
        
        session.sessionPreset = .photo
        
        do {
            let cameraInput = try AVCaptureDeviceInput(device: camera)
            session.addInput(cameraInput)
        } catch {
            fatalError(error.localizedDescription)
        }
        
        let videoOutput = AVCaptureVideoDataOutput()
        videoOutput.setSampleBufferDelegate(self, queue: dataOutputQueue)
        videoOutput.videoSettings = [kCVPixelBufferPixelFormatTypeKey as String: kCVPixelFormatType_32BGRA]
        
        session.addOutput(videoOutput)
        
        let videoConnection = videoOutput.connection(with: .video)
        videoConnection?.videoOrientation = .portrait
        
        let depthOutput = AVCaptureDepthDataOutput()
        depthOutput.setDelegate(self, callbackQueue: dataOutputQueue)
        depthOutput.isFilteringEnabled = true
        
        session.addOutput(depthOutput)
        
        let depthConnection = depthOutput.connection(with: .depthData)
        depthConnection?.videoOrientation = .portrait
        
        // Add photo output
        if session.canAddOutput(photoOutput) {
            session.addOutput(photoOutput)
            
            photoOutput.isHighResolutionCaptureEnabled = true
            
            //if depthVisualizationEnabled {
                if photoOutput.isDepthDataDeliverySupported {
                    photoOutput.isDepthDataDeliveryEnabled = true
                }
            //}
            
        } else {
            print("Could not add photo output to the session")
            session.commitConfiguration()
            return
        }
        
        let outputRect = CGRect(x: 0, y: 0, width: 1, height: 1)
        let videoRect = videoOutput.outputRectConverted(fromMetadataOutputRect: outputRect)
        let depthRect = depthOutput.outputRectConverted(fromMetadataOutputRect: outputRect)
        
        scale = max(videoRect.width, videoRect.height) / max(depthRect.width, depthRect.height)
        
        do {
            try camera.lockForConfiguration()
            
            if let frameDuration = camera.activeDepthDataFormat?
                .videoSupportedFrameRateRanges.first?.minFrameDuration {
                camera.activeVideoMinFrameDuration = frameDuration
            }
            
            camera.unlockForConfiguration()
        } catch {
            fatalError(error.localizedDescription)
        }
    }
}

// MARK: - Capture Video Data Delegate Methods

extension ViewController: AVCaptureVideoDataOutputSampleBufferDelegate {
    
    func captureOutput(_ output: AVCaptureOutput,
                       didOutput sampleBuffer: CMSampleBuffer,
                       from connection: AVCaptureConnection) {
        
        let pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer)
        let image = CIImage(cvPixelBuffer: pixelBuffer!)
        
        let previewImage: CIImage
        
        switch previewMode {
        case .original:
            previewImage = image
        case .depth:
            previewImage = depthMap ?? image
        
        }
        
        let displayImage = UIImage(ciImage: previewImage)
        DispatchQueue.main.async { [weak self] in
            self?.previewImage.image = displayImage
        }
    }
}

// MARK: - Capture Depth Data Delegate Methods

extension ViewController: AVCaptureDepthDataOutputDelegate {
    
    func depthDataOutput(_ output: AVCaptureDepthDataOutput,
                         didOutput depthData: AVDepthData,
                         timestamp: CMTime,
                         connection: AVCaptureConnection) {
        
        if previewMode == .original {
            return
        }
        
        var convertedDepth: AVDepthData
        
        if depthData.depthDataType != kCVPixelFormatType_DisparityFloat32 {
            convertedDepth = depthData.converting(toDepthDataType: kCVPixelFormatType_DisparityFloat32)
        } else {
            convertedDepth = depthData
        }
        
        let pixelBuffer = convertedDepth.depthDataMap
        //pixelBuffer.clamp()
        
        let depthMap = CIImage(cvPixelBuffer: pixelBuffer)
        

        
        DispatchQueue.main.async { [weak self] in
            self?.depthMap = depthMap
        }
    }
}

extension ViewController: AVCapturePhotoCaptureDelegate {
    func photoOutput(_ output: AVCapturePhotoOutput, didFinishProcessingPhoto photo: AVCapturePhoto, error: Error?) {
        guard let photoPixelBuffer = photo.pixelBuffer else {
            print("Error occurred while capturing photo: Missing pixel buffer (\(String(describing: error)))")
            return
        }
        
        var photoFormatDescription: CMFormatDescription?
        CMVideoFormatDescriptionCreateForImageBuffer(kCFAllocatorDefault, photoPixelBuffer, &photoFormatDescription)
        
        processingQueue.async {
            
            if let depthData = photo.depthData {
                let depthPixelBuffer = depthData.depthDataMap
                
                
                let metadataAttachments: CFDictionary = photo.metadata as CFDictionary
                guard let jpegData = ViewController.jpegData(withPixelBuffer: depthPixelBuffer, attachments: metadataAttachments) else {
                    print("Unable to create JPEG photo")
                    return
                }
            
            
            // Save JPEG to photo library
            PHPhotoLibrary.requestAuthorization { status in
                if status == .authorized {
                    PHPhotoLibrary.shared().performChanges({
                        let creationRequest = PHAssetCreationRequest.forAsset()
                        creationRequest.addResource(with: .photo, data: jpegData, options: nil)
                    }, completionHandler: { _, error in
                        if let error = error {
                            print("Error occurred while saving photo to photo library: \(error)")
                        }
                    })
                }
            }
            }
        }
    }
}


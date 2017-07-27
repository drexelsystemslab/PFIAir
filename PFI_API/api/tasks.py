from __future__ import absolute_import, unicode_literals
from celery import shared_task
from celery import group
from celery import chord
from django.core.files import File
from api.models import UserModel
from subprocess import call
import os
from pfitoolbox import ToolBox
import numpy as np
import math
import json
import time
import trimesh


@shared_task
def generatePreview(usermodelpk):
    try:
        print("Generating preview for usermodel: " + str(usermodelpk))
        userModel = UserModel.objects.get(pk=usermodelpk)
        print(userModel.file.url)
        filename = userModel.file.url.split('/')[-1]
        name = filename.split('.')[0]
        target_url = os.path.abspath(filename).rsplit("/",1)[0]+"/static/previews/"+name+".png"
        call(["blender",
              "--background",
              "--python",
              "api/management/commands/renderPreview.py",
              "--",
              userModel.file.url,
              target_url])

        previewFileName = name+'.png'
        with open('static/previews/'+previewFileName,'rb') as f:
            image_file = File(f)
            userModel.preview.save(previewFileName, image_file, save=False)
            userModel.save(update_fields=["preview"])
    except Exception as e:
        print(e.message)



@shared_task
def generateDescriptor(usermodelpk):
    userModel = UserModel.objects.get(pk=usermodelpk)

    fileName = userModel.file.url.split('/')[-1]
    print("Generating descriptor for usermodel: " + fileName)

    descriptorsChain = group([angleHistTask.s(usermodelpk),faceAreaHistTask.s(usermodelpk)])
    
    results = chord(descriptorsChain)(saveDescriptor.s(usermodelpk))
    #print(results.get())
    
    #userModel.descriptor.save(toolbox.getDescriptor())

    #generate = chain(*genDescriptorChain)
    #generate.delay()

    #TODO: need to make as indexed

@shared_task
def findNeighborsTask(usermodelpk):
    userModel = UserModel.objects.get(pk=usermodelpk)
    try:
        model = trimesh.load_mesh(userModel.file.url)
        return ToolBox.findNeighbors(model)
    except(OSError,IOError):
        print("31: stl file missing")
        return
    

@shared_task
def angleHistTask(usermodelpk):
    userModel = UserModel.objects.get(pk=usermodelpk)
    try:
        model = trimesh.load_mesh(userModel.file.url)
        return ToolBox.angleHist(model)
    except(OSError,IOError):
        print("31: stl file missing")
        return
    
@shared_task
def faceAreaHistTask(usermodelpk):
    userModel = UserModel.objects.get(pk=usermodelpk)
    try:
        model = trimesh.load_mesh(userModel.file.url)
        return ToolBox.faceAreaHist(model)
    except(OSError,IOError):
        print("31: stl file missing")
        return
    
@shared_task
def printResults(results):
    print(results)
    return results

@shared_task
def saveDescriptor(results,modelID):
    userModel = UserModel.objects.get(pk=modelID)
    print("Saving descriptor for: " + str(modelID))
    combined_descriptor = {}#the reformatted descriptor
    for descriptor in results:# [{'descriptor1':["something"]},{'descriptor1':["something"]}] -> {'descriptor1':["something"],'descriptor2':["something"]}
        combined_descriptor[descriptor.keys()[0]] = descriptor[descriptor.keys()[0]]
    

    print(combined_descriptor)
    userModel.descriptor = json.dumps(combined_descriptor)
    userModel.indexed = True
    userModel.save(update_fields=["indexed", "descriptor"])
    return results

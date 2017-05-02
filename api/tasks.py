from __future__ import absolute_import, unicode_literals
from celery import shared_task
from django.core.files import File
from api.models import UserModel
from subprocess import call
import os
import sys
from api.ToolBox import *
import numpy as np
import math
import json
import time

@shared_task
def generatePreview(usermodelpk):
	print("Generating preview for usermodel: " + str(usermodelpk))
	userModel = UserModel.objects.get(pk=usermodelpk)
	call(["blender", "--background","--python","api/management/commands/blenderObjToStl.py","--",userModel.file.url])
	filename = userModel.file.url.split('/')[-1]
	name = filename.split('.')[0]
	previewFileName = name+'.png'
	with open('static/previews/'+previewFileName,'rb') as f:
		image_file = File(f)
		userModel.preview.save(previewFileName,image_file,True)

# @shared_task
# def generateDescriptor(usermodelpk):
#     fileName = filepath.split('/')[-1]
#     print("Generating descriptor for usermodel: " + fileName)
#     userModel = UserModel.objects.get(pk=usermodelpk)
#     try:
#         model = mesh.Mesh.from_file(userModel.file.url)
#     except(OSError,IOError):
#         print("31: stl file missing")
#         return

# 	toolbox = ToolBox(True)

#     #Start generating task chain
#     genDescriptorChain = []
#     neighbors = toolbox.loadNeighborsGraph(fileName)#TODO: worker threads wouldn't have the file
#     if(len(model.points) != len(neighbors)):#we don't have a neighbors entry fro every point so something is wrong, let's regenreate the neighbor's graph
#         genDescriptorChain.append(tasks.findNeighbors)
#         indexes = len(model.points)
#         chunks= chunker(indexes,10)#break the list into 10 parts
#         genGraphWorkflow = chord((findNeighbors.s(model,chunk) for chunk in chunks),reducer.s())
#         genDescriptorChain.append(genGraphWorkflow)  


#     descriptorsChain = []

#     descriptorsChain.append(angleHist.s(neighbors))
#     genDescriptorChain.append(chord(group(*descriptorsChain),reducer.s()))#make the descriptors a group so they can be executed in parallel, then use a chord to merge them
#     userModel.descriptor.save(toolbox.getDescriptor())

#     generate = chain(*genDescriptorChain)
#     generate.delay()

    #TODO: need to make as indexed


def chunker(l, n):
    """Yield successive n-sized chunks from l."""
    for i in range(0, len(l), n):
        yield l[i:i + n]

def mag(x):
    return math.sqrt(x[0] ** 2 + x[1] ** 2 + x[2] ** 2)


def clean_acos(cos_angle):
    return math.acos(min(1, max(cos_angle, -1)))

@shared_task
def findNeighborsTask(model,indexes):#TODO: move out of this module
    return findNeighbors(model,indexes)

@shared_task
def reducer(lists):
    print(lists)
    results = []
    for l in lists:
        results += l
    return results #TODO: how can we cache this?

@shared_task
def angleHistTask(neighborsGraph):
    return angleHist(neighborsGraph)

#@shared_task(queue='IOQueue')
@shared_task
def printResults(results):
    print(results)
    return

#@shared_task(queue='IOQueue')
@shared_task
def saveNeighbors(neighborsGraph,modelID):
    userModel = UserModel.objects.get(pk=modelID)
    fileURL = userModel.file.url.rsplit('/',1)[0]
    filename = userModel.file.url.split('/')[-1]
    file = open(fileURL+"/"+filename+'_neighbors.pkl', 'wb')
    pickle.dump(neighborsGraph,file)
    return neighborsGraph #allow pass through for next task

#@shared_task(queue='IOQueue')
@shared_task
def saveDescriptor(descriptor,modelID):
    userModel = UserModel.objects.get(pk=modelID)
    userModel.descriptor = json.dumps(descriptor)
    userModel.indexed = True
    userModel.save()
    return descriptor

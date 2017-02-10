from __future__ import absolute_import, unicode_literals
from celery import shared_task
from django.core.files import File
from api.models import UserModel
from subprocess import call
import os

@shared_task
def add(x, y):
	with open("text.txt",'w') as f:
		f.write(x+y);
	return x + y

@shared_task
def generatePreview(usermodelpk):
	print("Generating preview for usermodel: " + str(usermodelpk))
	userModel = UserModel.objects.get(pk=usermodelpk)
	call(["blender", "--background","--python","api/management/commands/blenderObjToStl.py","--",userModel.file.url])
	filename = userModel.file.url.split('/')[-1]
	name = filename.split('.')[0]
	previewFileName = name+'.png'
	with open('static/previews/'+previewFileName,'r') as f:
		image_file = File(f)
		userModel.preview.save(previewFileName,image_file,True)
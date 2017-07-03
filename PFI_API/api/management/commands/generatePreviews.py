from django.core.management.base import BaseCommand, CommandError
from django.core.files import File
from api.models import UserModel
from django.conf import settings
from subprocess import call
import os

class Command(BaseCommand):
	help = "Creates the  preview images for all models";
	def handle(self, *args, **options):
		self.stdout.write("Start previewing")
		usermodels = UserModel.objects.all()
		for model in usermodels:
			self.stdout.write(model.file.url)
			call(["blender", "--background","--python","api/management/commands/blenderObjToStl.py","--",model.file.url])
			filename = model.file.url.split('/')[-1]
			name = filename.split('.')[0]
			previewFileName = name+'.png'
			with open('static/previews/'+previewFileName,'r') as f:
				image_file = File(f)
				model.preview.save(previewFileName,image_file,True)
			#os.remove('static/previews/'+name+'.png')# now in the database so we don't need it

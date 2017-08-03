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
		for userModel in usermodels:
			try:
				print(userModel.file.url)
				filename = userModel.file.url.split('/')[-1]
				name = filename.split('.')[0]
				target_url = os.path.abspath(filename).rsplit("/", 1)[0] + "/static/previews/" + name + ".png"
				call(["blender",
					  "--background",
					  "--python",
					  "api/management/commands/blenderObjToStl.py",
					  "--",
					  userModel.file.url,
					  target_url])

				previewFileName = name + '.png'
				with open('static/previews/' + previewFileName, 'rb') as f:
					image_file = File(f)
					userModel.preview.save(previewFileName, image_file, save=False)
					userModel.save(update_fields=["preview"])
			except Exception as e:
				print(e.message)

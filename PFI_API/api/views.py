from django.shortcuts import render
from django.http import HttpResponse
from django.http import JsonResponse
from django.http import HttpResponseRedirect
from django.core import serializers
from django.conf import settings
from models import UserModel
from models import UserModelForm
from forms import SearchForm
import random
import numpy as np
import mimetypes
import os
import sys
from pfitoolbox import ToolBox
import tasks
from celery import group
import json
import trimesh

def index(request):
	return HttpResponse("Hello, world. You're at the index.")

def upload(request):
	if request.method == "POST":
		form = UserModelForm(request.POST,request.FILES)
		if form.is_valid():
			newUserModel = form.save()
			# model = trimesh.load_mesh(newUserModel.file.url)
			# descriptor = ToolBox.angleHist(model)
			# newUserModel.descriptor = json.dumps(descriptor)
			# newUserModel.indexed = True
			# newUserModel.save()
			#tasks.generatePreview(newUserModel.pk)
			tasks.generatePreview.delay(newUserModel.pk)#send the pk instead of the object to prevent race conditions
			tasks.generateDescriptor.delay(newUserModel.pk)
			return HttpResponseRedirect('/usermodels')
	else:
		form = UserModelForm()
	return render(request, 'upload.html', {'form':form})

def getUserModel(request):
	userModels = serializers.serialize('python',UserModel.objects.all())
	return render(request, 'UserModelList.html', {"userModel_list":userModels})

def search(request):
	if(request.method == "POST"):
		#The user is submitting a file to search
		#let's get the file from the request
		form = SearchForm(request.POST, request.FILES)
		if(form.is_valid()): #TODO: check for legal file types
			form_id = ''.join([random.choice('1234567890qwertyuiopasdfghjklzxcvbnm') for i in range(7)])
			handle_uploaded_file(request.FILES['userModel'],form_id)

			#ok now the file is in temp/[form_id].um (stands for user model)
			#so let's generate it's descriptor
			model = trimesh.load_mesh('temp/'+form_id+'.stl')
			
			angleHist = np.array(ToolBox.angleHist(model)['angleHist'])#strip the descriptor of its label and convert it to a numpy array
			faceAreaHist = np.array(ToolBox.faceAreaHist(model)['faceAreaHist'])
			
			newUserModelAngleHist = angleHist
			usermodels = UserModel.objects.filter(indexed=True)#get all indexed models in the database
			models = []
			for userModel in usermodels:
				descriptor = json.loads(userModel.descriptor)
				angleHist = np.array(descriptor['angleHist'])
				distance = np.linalg.norm(newUserModelAngleHist[:,1]-angleHist[:,1])#because the data is [lable,value] we need to just subtract values
				models.append({"pk":userModel.pk,"distance":distance})

			print(models)
			topsix = sorted(models, key=lambda i:i["distance"])[0:6]
			results = []
			print(topsix)
			for result in topsix:
				matchedUserModels = serializers.serialize('python',UserModel.objects.filter(pk=result["pk"]))
				results.append(matchedUserModels[0])
			return render(request, 'UserModelList.html', {"userModel_list":results})
			

			return HttpResponse("thank you for uploading")
		else:
			return HttpResponse("FORM Error")
	else:
		form = SearchForm()
		return render(request, 'Search.html', {'form':form})

def download(request,file_pk):
	usermodel = UserModel.objects.get(pk=file_pk)
	file_path = usermodel.file.url #TODO: why is there an extra /uploads/
	file_name = usermodel.file.url.split('/')[-1]
	print(file_path)
	file_wrapper = open(file_path,'rb')
	file_mimetype = mimetypes.guess_type(file_path)
	response = HttpResponse(file_wrapper, content_type=file_mimetype)
	response['X-Sendfile'] = file_path
	response['Content-Length'] = os.stat(file_path).st_size
	response['Content-Disposition'] = 'attachment; filename=%s' % file_name
	return response

def delete(request,file_pk):
	data = {"success":False}
	if(request.method == "DELETE"):
		usermodel = UserModel.objects.get(pk=file_pk)
		usermodel.delete()
		data["success"] = True

	return JsonResponse(data)

def handle_uploaded_file(f,form_id):
	with open("temp/"+form_id+'.stl','wb+') as destination:
		for chunk in f.chunks():
			destination.write(chunk)
	pass

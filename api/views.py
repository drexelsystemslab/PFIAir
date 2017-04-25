from django.shortcuts import render
from django.http import HttpResponse
from django.http import JsonResponse
from django.http import HttpResponseRedirect
from django.core import serializers
from django.conf import settings
from api.models import UserModel
from api.models import UserModelForm
from api.forms import SearchForm
import random
import numpy as np
import pickle
import mimetypes
import os 
from api import ToolBox
from stl import mesh
from api import tasks
from celery import chord
from celery import group
from celery import chain
import json

def index(request):
    return HttpResponse("Hello, world. You're at the index.")

def upload(request):
	if request.method == "POST":
		form = UserModelForm(request.POST,request.FILES)
		if form.is_valid():
			newUserModel = form.save()
			tasks.generatePreview.delay(newUserModel.pk)#send the pk instead of the object to prevent race conditions

			stlmodel = mesh.Mesh.from_file(newUserModel.file.url)
			model = {"points":stlmodel.points.tolist(),"normals":stlmodel.normals.tolist()}
			genDescriptorChain = []

			indexes = range(0,len(model["points"]))
			chunks= ToolBox.chunker(indexes,1000)#break the list into 10 parts
			genGraphWorkflow = chord((tasks.findNeighborsTask.s(model,chunk) for chunk in chunks),tasks.reducer.s())
			genDescriptorChain.append(genGraphWorkflow)

			genDescriptorChain.append(tasks.saveNeighbors.s(newUserModel.pk))

			descriptorsChain = []

			#descriptorsChain.append(angleHistTask.s())
			#descriptorsWorkflow = chord(group(*descriptorsChain),reducer.s())
			#genDescriptorChain.append(descriptorsWorkflow)#make the descriptors a group so they can be executed in parallel, then use a chord to merge them

			genDescriptorChain.append(task.angleHistTask.s())

			genDescriptorChain.append(tasks.saveDescriptor.s(newUserModel.pk))
			generate = chain(*genDescriptorChain)
			result = generate.delay()

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
			stlmodel = mesh.Mesh.from_file('temp/'+form_id+'.um')##TODO: these should be a new type of models so we can track them
			model = {"points":stlmodel.points.tolist(),"normals":stlmodel.normals.tolist()}
			genDescriptorChain = []

			indexes = range(0,len(model["points"]))
			chunks= ToolBox.chunker(indexes,1000)#break the list into 10 parts
			genGraphWorkflow = chord((findNeighborsTask.s(model,chunk) for chunk in chunks),reducer.s())
			genDescriptorChain.append(genGraphWorkflow)

			descriptorsChain = []

			#descriptorsChain.append(angleHistTask.s())
			#descriptorsWorkflow = chord(group(*descriptorsChain),reducer.s())
			#genDescriptorChain.append(descriptorsWorkflow)#make the descriptors a group so they can be executed in parallel, then use a chord to merge them

			genDescriptorChain.append(angleHistTask.s())
			generate = chain(*genDescriptorChain)
			process = generate.delay()

			start = time.time()
			while (process.ready() == False):
				print(time.time()-start)
				time.sleep(1)

			newUserModelDescriptor = process.get(timeout=1)#don't need to json.loads because the process returns a python array already
			newUserModelAngleHist = np.array(newUserModelDescriptor[1])#strip off the lable to get at the data
			usermodels = UserModel.objects.filter(indexed=True)
			models = []
			for userModel in usermodels:
				descriptor = json.loads(userModel.descriptor)
				angleHist = np.array(descriptor[1])
				distance = np.linalg.norm(newUserModelAngleHist[:,1]-angleHist[:,1])#because the data is [lable,value] we need to just subtract values
				models.append({"pk":userModel.pk,"distance":distance})

			print(models)
			topthree = sorted(models, key=lambda i:i["distance"])[0:3]
			results = []
			print(topthree)
			for result in topthree:
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
	with open("temp/"+form_id+'.um','wb+') as destination:
		for chunk in f.chunks():
			destination.write(chunk)
	pass

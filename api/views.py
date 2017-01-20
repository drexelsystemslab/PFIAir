from django.shortcuts import render
from django.http import HttpResponse
from django.http import JsonResponse
from django.core import serializers
from api.models import UserModel
from api.models import UserModelForm
from api.forms import SearchForm
import random
from sklearn.feature_extraction.text import TfidfVectorizer
import numpy as np
import pickle
import mimetypes
import os 

def index(request):
    return HttpResponse("Hello, world. You're at the polls index.")

def upload(request):
	if request.method == "POST":
		form = UserModelForm(request.POST,request.FILES)
		if form.is_valid():
			form.save()
			return HttpResponse("Thank you for uploading")
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
			#TFIDF implementation
			tfidf = pickle.load(open('tfidf.pkl',"rb"))
			usermodels = UserModel.objects.filter(indexed=True)
			documents = []
			docPks = [] #Im using lists because I want to maintain order. Ideally this would be a dictionary
			for userModel in usermodels:
				documents.append(userModel.file.url.split('/',2)[2])
				docPks.append(userModel.pk)

			documents.append("temp/"+form_id+".um")#add the new file
				
			tdm = tfidf.transform(documents)
			closeness = np.array((tdm*tdm.T).A)
			#print closeness
			#print closeness[:-1,-1]
			topthree = sorted(range(len(closeness[:-1,-1])), key=lambda i:closeness[:-1,-1][i],reverse=True)[0:3]
			results = []
			for result in topthree:
				matchedUserModels = serializers.serialize('python',UserModel.objects.filter(pk=result))
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
	file_path = usermodel.file.url.split('/',2)[2] #TODO: why is there an extra /uploads/
	file_name = usermodel.file.url.split('/')[-1]
	file_wrapper = open(file_path,'rb')
	file_mimetype = mimetypes.guess_type(file_path)
	response = HttpResponse(file_wrapper, content_type=file_mimetype)
	response['X-Sendfile'] = file_path
	response['Content-Length'] = os.stat(file_path).st_size
	response['Content-Disposition'] = 'attachment; filename=%s' % file_name
	return response

def handle_uploaded_file(f,form_id):
	with open("temp/"+form_id+'.um','wb+') as destination:
		for chunk in f.chunks():
			destination.write(chunk)
	pass
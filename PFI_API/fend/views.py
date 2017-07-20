from django.shortcuts import render
from django.http import HttpResponseNotAllowed, HttpResponseBadRequest, HttpResponseRedirect
from fend.forms import SearchForm
from django.core import serializers
from api.models import UserModel
from django.views.decorators.csrf import csrf_exempt
import random
import numpy as np
import trimesh
import json
from pfitoolbox import ToolBox
from fend.forms import SearchForm
from api.models import UserModelForm
import tasks

def getUserModels(request):
    userModels = serializers.serialize('python', UserModel.objects.all())
    return render(request, 'UserModelList.html', {"userModel_list": userModels})

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

@csrf_exempt
def search(request):
    if (request.method == "POST"):
        # The user is submitting a file to search
        # let's get the file from the request
        form = SearchForm(request.POST, request.FILES)
        if (form.is_valid()):  # TODO: check for legal file types
            form_id = ''.join([random.choice('1234567890qwertyuiopasdfghjklzxcvbnm') for i in range(7)])
            handle_uploaded_file(request.FILES['file'], form_id)

            # ok now the file is in temp/[form_id].um (stands for user model)
            # so let's generate it's descriptor
            model = trimesh.load_mesh('temp/' + form_id + '.stl')

            angleHist = np.array(ToolBox.angleHist(model)[
                                     'angleHist'])  # strip the descriptor of its label and convert it to a numpy array
            faceAreaHist = np.array(ToolBox.faceAreaHist(model)['faceAreaHist'])

            newUserModelAngleHist = angleHist
            usermodels = UserModel.objects.filter(indexed=True)  # get all indexed models in the database
            models = []
            for userModel in usermodels:
                descriptor = json.loads(userModel.descriptor)
                angleHist = np.array(descriptor['angleHist'])
                distance = np.linalg.norm(newUserModelAngleHist[:, 1] - angleHist[:,
                                                                        1])  # because the data is [lable,value] we need to just subtract values
                models.append({"pk": userModel.pk, "distance": distance})

            topsix = sorted(models, key=lambda i: i["distance"])[0:6]
            results = []
            print(topsix)
            for result in topsix:
                matchedUserModels = serializers.serialize('python', UserModel.objects.filter(pk=result["pk"]))
                matchedUserModels[0]["distance"] = result["distance"]
                print(matchedUserModels)
                results.append(matchedUserModels[0])

            return render(request, 'UserModelList.html', {"userModel_list": results})
        else:
                return HttpResponseBadRequest()
    else:
        form = SearchForm()
        return render(request, 'Search.html', {'form': form})

def handle_uploaded_file(f, form_id):
    with open("temp/" + form_id + '.stl', 'wb+') as destination:
        for chunk in f.chunks():
            destination.write(chunk)
    pass
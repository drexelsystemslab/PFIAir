from django.shortcuts import render
from django.core.paginator import Paginator, EmptyPage, PageNotAnInteger
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
    userModels = UserModel.objects.all()
    paginator = Paginator(userModels, 15)  # Show 25 contacts per page

    page = request.GET.get('page')
    try:
        userModels_subset = paginator.page(page)
    except PageNotAnInteger:
        # If page is not an integer, deliver first page.
        userModels_subset = paginator.page(1)
    except EmptyPage:
        # If page is out of range (e.g. 9999), deliver last page of results.
        userModels_subset = paginator.page(paginator.num_pages)
    #userModels_python = serializers.serialize('python', userModels_subset)
    return render(request, 'UserModelList.html', {"userModel_list": userModels_subset})




def upload(request):
    if request.method == "POST":
        form = UserModelForm(request.POST, request.FILES)
        if form.is_valid():
            # if (request.FILES["file"].content_type == 'application/vnd.ms-pki.stl'):
            #     pass
            # else:
            #     return HttpResponseBadRequest("Invalid File Format %s" % request.FILES["file"].content_type)
            newUserModel = form.save()
            # model = trimesh.load_mesh(newUserModel.file.url)
            # descriptor = ToolBox.angleHist(model)
            # newUserModel.descriptor = json.dumps(descriptor)
            # newUserModel.indexed = True
            # newUserModel.save()
            # tasks.generatePreview(newUserModel.pk)
            tasks.generatePreview.delay(newUserModel.pk)  # send the pk instead of the object to prevent race conditions
            tasks.generateDescriptor.delay(newUserModel.pk)
            return HttpResponseRedirect('/usermodels')
        else:
            errors = ""
            for field in form:
                errors += field.errors
            return HttpResponseBadRequest(errors)

    else:
        form = UserModelForm()
        return render(request, 'upload.html', {'form': form})


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

from django.shortcuts import render
from django.core.paginator import Paginator, EmptyPage, PageNotAnInteger
from django.http import HttpResponseNotAllowed, HttpResponseBadRequest, HttpResponseRedirect, HttpResponseServerError
from django.db import IntegrityError
from django.forms import ValidationError
from fend.forms import SearchForm
from django.core import serializers
from api.models import UserModel
from api.models import File
from django.views.decorators.csrf import csrf_exempt
import random
import numpy as np
import trimesh
import json
from pfitoolbox import ToolBox
from fend.forms import SearchForm
from fend.forms import UploadForm
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
        form = UploadForm(request.POST,request.FILES)
        try:
            if form.is_valid():
                newUserModel = UserModel(name=form.cleaned_data['name'])
                newUserModel.save()
                file = File(file=request.FILES['file'],model=newUserModel)
                file.save()

                tasks.generatePreview.delay(newUserModel.pk)  # send the pk instead of the object to prevent race conditions
                tasks.generateDescriptor.delay(newUserModel.pk)
                return HttpResponseRedirect('/usermodels')


        except ValidationError:
            error_messages = ""
            for (field, errors) in form.errors.as_data().iteritems():
                for error in errors:
                    error_messages += "%s: %s\n" % (field, error.message)
            return HttpResponseBadRequest(error_messages)
        except Exception as e:
            return HttpResponseServerError(str(e))

    else:
        form = UploadForm()
        print(form)
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
                matchedUserModel = UserModel.objects.get(pk=result["pk"])
                #print(matchedUserModels)
                results.append((matchedUserModel,result["distance"]))
            print(results)
            return render(request, 'SearchResults.html', {"userModel_list": results})
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

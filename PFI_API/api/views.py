import json
import mimetypes
import os
import random
import numpy as np
import trimesh
from django.core import serializers
from django.http import HttpResponse, JsonResponse, HttpResponseBadRequest, HttpResponseServerError, Http404
from django.views.decorators.csrf import csrf_exempt
from django.views.decorators.http import require_http_methods

from pfitoolbox import ToolBox

import tasks
from fend.forms import SearchForm
from api.models import UserModel
from api.models import UserModelForm

@csrf_exempt
@require_http_methods(["GET","POST"])
def models(request):
    if(request.method == "GET"):
        try:
            models = UserModel.objects.only('id', 'name', 'preview', 'file')
            userModels = serializers.serialize('python', models)
            return JsonResponse(models_to_json(userModels))
        except Exception as e:
            return HttpResponseServerError(str(e))
    elif(request.method == "POST"):
        form = UserModelForm(request.POST, request.FILES)
        if form.is_valid():
            if(request.FILES["file"].content_type == 'application/vnd.ms-pki.stl'):
                pass
            else:
                return HttpResponseBadRequest("Invalid File Format %s" % request.FILES["file"].content_type)

            newUserModel = form.save()
            tasks.generatePreview.delay(newUserModel.pk)  # send the pk instead of the object to prevent race conditions
            tasks.generateDescriptor.delay(newUserModel.pk)
            return JsonResponse({"success": True})
        else:
            return HttpResponseBadRequest("Invalid Form")
    else:
        return HttpResponseBadRequest("Invalid Form")

@csrf_exempt
@require_http_methods(["POST"])
def search(request):
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
        distances = []
        for userModel in usermodels:
            descriptor = json.loads(userModel.descriptor)
            angleHist = np.array(descriptor['angleHist'])
            distance = np.linalg.norm(newUserModelAngleHist[:, 1] - angleHist[:,1])  # because the data is [lable,value] we need to just subtract values
            models.append({"pk":userModel.pk,"distance":distance})

        topsix = sorted(models, key=lambda i: i["distance"])[0:6]
        results = []
        for result in topsix:
            matchedUserModel = serializers.serialize('python',UserModel.objects.filter(pk=result["pk"]))

            modelObject = models_to_json(matchedUserModel)["models"][0]
            modelObject["distance"] = result["distance"]
            #modelObject["distance"] = result["distance"]
            results.append(modelObject)

        print(results)
        try:
            return JsonResponse({"models":results})
        except Exception as e:
            response = JsonResponse({'error': e.message})
            response.status_code = 400
            return response
    else:
        return HttpResponseBadRequest()

@csrf_exempt
@require_http_methods(["POST"])
def download(request, file_pk):
    usermodel = UserModel.objects.get(pk=file_pk)
    file_path = usermodel.file.url  # TODO: why is there an extra /uploads/
    file_name = usermodel.file.url.split('/')[-1]
    print(file_path)
    file_wrapper = open(file_path, 'rb')
    file_mimetype = mimetypes.guess_type(file_path)
    response = HttpResponse(file_wrapper, content_type=file_mimetype)
    response['X-Sendfile'] = file_path
    response['Content-Length'] = os.stat(file_path).st_size
    response['Content-Disposition'] = 'attachment; filename=%s' % file_name
    return response

@csrf_exempt
@require_http_methods(["DELETE"])
def delete(request, file_pk):
    data = {"success": False}
    try:
        usermodel = UserModel.objects.get(pk=file_pk)
        usermodel.delete()
        data["success"] = True
        return JsonResponse(data)
    except UserModel.DoesNotExist:
        return Http404()


def handle_uploaded_file(f, form_id):
    with open("temp/" + form_id + '.stl', 'wb+') as destination:
        for chunk in f.chunks():
            destination.write(chunk)
    pass


def models_to_json(models):
    results = {'models': []}
    for model in models:
        results['models'].append({
            "name": model['fields']['name'],
            "id": model['pk'],
            "preview": model['fields']['preview'],
            "location": "/download/" + str(model['pk'])
        })
    return results

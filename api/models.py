from __future__ import unicode_literals
from django.db import models
from django import forms

class UserModel(models.Model):
	#implicate ID field
	name = models.TextField()
	file = models.FileField(upload_to='uploads/')
	indexed = models.BooleanField(default=False)

class UserModelForm(forms.ModelForm):
    class Meta:
        model = UserModel
        fields = ['name','file']

class ModelIndex(models.Model):
	usermodel = models.OneToOneField(UserModel,on_delete=models.CASCADE,primary_key=True)
	tree = models.TextField()
		
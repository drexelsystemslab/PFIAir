from __future__ import unicode_literals
from django.db import models
from django.db.models.signals import pre_save
from django.dispatch import receiver
from django import forms
from .validators import validate_file_extension
import os


class UserModel(models.Model):
    # implicate ID field
    name = models.TextField()
    indexed = models.BooleanField(default=False)
    descriptor = models.TextField(default="[]", null=True, blank=True)
    preview = models.ImageField(upload_to='static/previews/', default=None)

    def filename(self):
        return os.path.basename(self.file.name)

    def previewfilename(self):
        return os.path.basename(self.preview.name)

class UserModelForm(forms.ModelForm):
    class Meta:
        model = UserModel
        fields = ['name']

class File(models.Model):
    file = models.FileField(upload_to='models/',validators=[validate_file_extension])
    type = models.TextField(null=False,default=None)
    model = models.ForeignKey(UserModel, on_delete=models.SET_NULL, related_name='file',null=True)

class FileForm(forms.ModelForm):
    class Meta:
        model = File
        fields = ['file']


class ModelIndex(models.Model):
    usermodel = models.OneToOneField(UserModel, on_delete=models.CASCADE, primary_key=True)
    tree = models.TextField()

@receiver(pre_save, sender=File)
def set_file_type(sender,instance, **kwargs):
    '''when the file is saved, set file type=the file extension'''
    instance.type = instance.file.name.split('.')[-1]
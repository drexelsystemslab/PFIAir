from __future__ import unicode_literals
from django.db import models
from django import forms


class UserModel(models.Model):
    # implicate ID field
    name = models.TextField()
    file = models.FileField(upload_to='models/')
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
        fields = ['name', 'file']


class ModelIndex(models.Model):
    usermodel = models.OneToOneField(UserModel, on_delete=models.CASCADE, primary_key=True)
    tree = models.TextField()

from __future__ import unicode_literals
from django.db import models
from django import forms
from django.contrib.postgres.fields import ArrayField,JSONField
from .validators import validate_file_extension
import os


class UserModel(models.Model):
    # implicate ID field
    name = models.TextField()
    file = models.FileField(upload_to='models/',validators=[validate_file_extension])
    indexed = models.BooleanField(default=False)
    descriptor = models.TextField(default="[]", null=True, blank=True)
    preview = models.ImageField(upload_to='static/previews/', default=None)
    tree = JSONField(null=True)

    def filename(self):
        return os.path.basename(self.file.name)

    def previewfilename(self):
        return os.path.basename(self.preview.name)


class UserModelForm(forms.ModelForm):
    class Meta:
        model = UserModel
        fields = ['name', 'file']


class UserModelNode(models.Model):
    parent = models.ForeignKey(UserModel, on_delete=models.CASCADE)
    model = JSONField(default={'metadata': [],'faces': [],'face_normals': [],'vertices': []})
    descriptor = ArrayField(models.FloatField(),blank=True)

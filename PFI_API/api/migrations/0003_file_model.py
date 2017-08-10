# -*- coding: utf-8 -*-
# Generated by Django 1.11.3 on 2017-08-10 18:16
from __future__ import unicode_literals

from django.db import migrations, models
import django.db.models.deletion


class Migration(migrations.Migration):

    dependencies = [
        ('api', '0002_remove_file_model'),
    ]

    operations = [
        migrations.AddField(
            model_name='file',
            name='model',
            field=models.ForeignKey(null=True, on_delete=django.db.models.deletion.SET_NULL, related_name='file', to='api.UserModel'),
        ),
    ]

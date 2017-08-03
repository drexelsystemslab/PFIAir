from django.core.management.base import BaseCommand, CommandError
from django.core.files import File
from api.models import UserModel
from django.conf import settings
from subprocess import call
import os


class Command(BaseCommand):
    help = "checks database for missing previews or indexes";

    def handle(self, *args, **options):
        self.stdout.write("Non-indexed models")
        usermodels = UserModel.objects.filter(indexed=0, descriptor="{}")

        for userModel in usermodels:
            print("%s: %s" % (userModel.pk, userModel.file.url))

        print("No preview models")
        usermodels = UserModel.objects.filter(preview="")
        for userModel in usermodels:
            print("%s: %s" % (userModel.pk, userModel.file.url))

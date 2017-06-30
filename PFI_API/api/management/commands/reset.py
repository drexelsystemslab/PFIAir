from django.core.management.base import BaseCommand, CommandError
from api.models import UserModel

class Command(BaseCommand):
    help = 'Marks all usermodels as not indexed'

    def handle(self, *args, **options):
        usermodels = UserModel.objects.all()
        for usermodel in usermodels:
        	usermodel.indexed = False
        	usermodel.save()

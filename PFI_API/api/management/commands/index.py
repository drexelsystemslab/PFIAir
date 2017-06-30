from django.core.management.base import BaseCommand, CommandError
from sklearn.feature_extraction.text import TfidfVectorizer
from api.models import UserModel
import pickle


class Command(BaseCommand):
	help = "Creates search index"
	
	def handle(self, *args, **options):
		newUserModels = UserModel.objects.filter(indexed=False)#get all usermodels that have not been indexed
		self.stdout.write("length:"+str(len(newUserModels)))
		documents = []
		for userModel in newUserModels:
			try:
				documents.append(userModel.file.url)
				#mark as indexed
				userModel.indexed = True
				userModel.save()
			except IOError:
				self.stdout.write("Error opening file " + userModel.file)
		tfidf = TfidfVectorizer(input="filename").fit(documents)
		pickle.dump(tfidf, open( 'tfidf.pkl', "wb" ) )	
	
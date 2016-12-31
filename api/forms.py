from django import forms

class SearchForm(forms.Form):
	userModel = forms.FileField(label="") #I dont want a label

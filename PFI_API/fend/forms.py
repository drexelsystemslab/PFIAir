from django import forms

class SearchForm(forms.Form):
	file = forms.FileField(label="") #I dont want a label

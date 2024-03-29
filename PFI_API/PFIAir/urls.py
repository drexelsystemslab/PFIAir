"""PFIAir URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/1.10/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  url(r'^$', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  url(r'^$', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.conf.urls import url, include
    2. Add a URL to urlpatterns:  url(r'^blog/', include('blog.urls'))
"""
from django.conf.urls import url
from django.contrib import admin
from api import views as api_views
from fend import views as fend_views
from django.conf import settings
from django.conf.urls.static import static

urlpatterns = [
    #url(r'^admin/', admin.site.urls),
    url(r'^$', fend_views.getUserModels, name='User Models'),
    url(r'^upload/$', fend_views.upload, name='upload'),
    url(r'^search/$', fend_views.search, name='apiSearch'),
    url(r'^api/models/$', api_views.models, name='apiGetUserModel'),
    url(r'^api/search/$', api_views.search, name='search'),
    url(r'^api/download/(?P<file_pk>.+)$',api_views.download,name='download'),
    url(r'^api/delete/(?P<file_pk>.+)$',api_views.delete,name='delete')
] + static(settings.STATIC_URL, document_root=settings.STATIC_ROOT)

# copied from the internet because it might be useful in the future
# urlpatterns += patterns('',
#         url(r'^media/(?P<path>.*)$', 'django.views.static.serve', {
#             'document_root': settings.MEDIA_ROOT,
#         }),
#         url(r'^static/(?P<path>.*)$', 'django.views.static.serve', {
#             'document_root': settings.STATIC_ROOT,
#         }),
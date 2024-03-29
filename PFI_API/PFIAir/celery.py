from __future__ import absolute_import, unicode_literals
import os
from celery import Celery

# set the default Django settings module for the 'celery' program.
os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'PFIAir.settings')

app = Celery('proj',broker='amqp://',backend='amqp')

# Using a string here means the worker don't have to serialize
# the configuration object to child processes.
# - namespace='CELERY' means all celery-related configuration keys
#   should have a `CELERY_` prefix.
app.config_from_object('django.conf:settings')

# Load task modules from all registered Django app configs.
app.autodiscover_tasks()

# add a new queue to seperate tasks that require file IO
#app.control.add_consumer('IOQueue', reply=True)

@app.task(bind=True)
def debug_task(self):
    print('Request: {0!r}'.format(self.request))

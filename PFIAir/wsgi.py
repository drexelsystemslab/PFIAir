"""
WSGI config for PFIAir project.

It exposes the WSGI callable as a module-level variable named ``application``.

For more information on this file, see
https://docs.djangoproject.com/en/1.10/howto/deployment/wsgi/
"""

import os
import sys
import logging, sys

from django.core.wsgi import get_wsgi_application

logging.basicConfig(stream=sys.stderr)
print(sys.prefix, file=sys.stderr)

os.environ.setdefault("DJANGO_SETTINGS_MODULE", "PFIAir.settings")

application = get_wsgi_application()

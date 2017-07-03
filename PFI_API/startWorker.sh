source activate py2.7
celery -A PFIAir worker -l DEBUG -n worker1@%h

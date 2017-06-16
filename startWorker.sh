<<<<<<< Updated upstream
source activate py35
celery -A PFIAir worker -l DEBUG -n worker1@%h

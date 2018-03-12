FROM tiangolo/uwsgi-nginx-flask:python3.6
ENV UWSGI_INI uwsgi.ini
COPY requirments.txt requirments.txt
RUN pip install -r requirments.txt
COPY ./app /app

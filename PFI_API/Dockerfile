FROM continuumio/anaconda

RUN apt-get update && apt-get install -y gcc libpq-dev

ADD requirments.txt /app/requirments.txt

WORKDIR /app/

RUN pip install -r requirments.txt

RUN adduser --disabled-password --gecos '' myuser

WORKDIR /app
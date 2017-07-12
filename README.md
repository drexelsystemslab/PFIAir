# PFIAir Installation

## Installation requirements
This project does not run on MS Windows. Minmium requirements are as follows, please check individual project pages for other requirements.
Installation instructions will be written for Ubuntu, and may differ on other platforms.

- Python 2.7
- Blender 2.78
- Celery 4.0.2
- Anaconda 4.4.0 (python 2.7 version)

## Installing dependencies
### Andaconda
Download the proper version of Anaconda from [this page](https://www.continuum.io/downloads). Open the download directory and run the following command:

```
bash <insert Anaconda installer file name here>
```

Following the prompts, install Anaconda in the default directory and append Anaconda to your PATH. Close your terminal and re-open so you can run "conda" commands

### Python
We will be creating a virtual enviroment for the project named "py2.7". To do this, run the following commands:

```
conda create -n py2.7 python=2.7 anaconda
```

Activate this enviroment with

```
source activate py2.7
```

Please use this enviroment to install the remaining dependancies. This enviroment can be deactivated using the following command, or by closing your terminal

```
source deactivate py2.7
```

### Celery

Check out [this page](http://docs.celeryproject.org/en/latest/getting-started/first-steps-with-celery.html#installing-celery) for instructions on installing Celery 4
This project uses the RabbitMQ broker, so please use that part of the installation instructions

### Blender

Check out [this page](https://www.blender.org/download/) for the download link appropriate for your system.
In the download directory, run the command
```
tar -xjf <blender install file>
sudo cp <unziped directory> /usr/lib/blender -r
[Add blender to path]

```

[change default scene]

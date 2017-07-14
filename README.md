# PFIAir Installation

## Installation requirements
This project does not run on MS Windows. Software requirements are as follows, please check individual project pages for other requirements.
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
For Mac: http://docs.celeryproject.org/en/latest/getting-started/first-steps-with-celery.html#installing-celery
If you have homebrew installed: use the 
```
brew install rabbitmq-server
```
Then paste path for rabbitmq (/usr/local/sbin) to /etc/paths:
```
vi /etc/paths
```

### Blender

Check out [this page](https://www.blender.org/download/) for the download link appropriate for your system.
In the download directory, run the command
```
tar -xjf <blender install file>
mv <unzipped directory> blender
sudo mv blender /usr/lib/
```

Then add `/usr/lib/blender` to your path.

There is a startup.blend file in the repo to ensure that all preview images that are generated look the same. To install this copy the file from `PFIAir/PFI_Blender/` to `/home/<UserName>/.config/blender/2.78/config`. (Mac: `/home/<UserName>/Library/Application Support/Blender/2.78/config`)

For Mac: Paste path for blender.exe file to /etc/paths using:
```
vi /etc/paths
```



### pfitoolbox

### Additional Python Dependancies

There are a few other libraries required by the project. To install them, run the following commands:
```
source activate py2.7
pip install django
pip install trimesh
```

Additionally, please installthe python library associated with this project. In the `PFIAir/pfitoolbox` directory run the following command:

```
source activate py2.7
pip install -e .
```

## Running the server
The server has two parts: a Celery worker to handle sycronous tasks, and a Django server to provide UI. To start the worker, change to the `PFIAir/PFI_API` directory and run the `startWorker.sh` script.

Once that has started, open a second command window, change to the `PFIAir/PFI_API` directory, and run the `startServer.sh` script. If this is your first time running the server, you will need to run the follwoing commands:

For Mac, also run the command `rabbitmq-server`


```
source activate py2.7
python manage.py migrate
```

At this point you should be able to browse to `127.0.0.1:8000` and see that the server is running.

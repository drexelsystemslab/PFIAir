from io import BytesIO
import requests

username = "user1"
password = "thf2gxHafPbKT"
url = "http://api.3dindustri.es/db2_145/"

f = requests.get(url, auth=(username, password))
html = f.text

code = open("models.txt","w")
code.write(html)
code.close()

with open("models.txt") as r:
    data = r.readlines()

i = 0
obj = 0
threeds = 0
for i in range(11,192):
    id = data[i][80:87]
    res = requests.get(url + "/%s/model/%s.obj" % (id,id), auth=(username,password))
    if res.status_code == 200:
        if (requests.post("http://127.0.0.1:8000/model/",data={"name":id+".obj","file":BytesIO(res.content)})).status_code != 200:

            print(requests.post("http://127.0.0.1:8000/model/", data={"name":id+".obj","file":BytesIO(res.content)}))
            print(str(id) + " didn't upload")
        else:
            print('WORKED')
        continue
    res = requests.get(url + "/%s/model/%s.3ds" % (id,id), auth=(username,password))
    if res.status_code == 200:
        if (requests.post("http://127.0.0.1:8000/model", data={"name":id+".3ds","file":BytesIO(res.content)})).status_code != 200:

            print(requests.post("http://127.0.0.1:8000/model", data={"name":id+".3ds","file":BytesIO(res.content)}))
            print(str(id) + " didn't upload")
        else:
            print('WORKED')





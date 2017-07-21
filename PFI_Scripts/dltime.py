import requests
import time
start_time = time.time()

username = "user1"
password = "thf2gxHafPbKT"
url = "http://api.3dindustri.es/db2_145/m145953/model/m145953.3ds"

f = requests.get(url, auth=(username, password))

print("--- %s seconds ---" % (time.time() - start_time))
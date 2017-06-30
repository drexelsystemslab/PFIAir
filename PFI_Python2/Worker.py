import math
import numpy as np
import multiprocessing
import time
import pickle

class Worker:
    def __init__(self,model):
        self.model = model
    def mag(self,x):
        return math.sqrt(x[0]**2+x[1]**2+x[2]**2)
    def clean_acos(self,cos_angle):
        return math.acos(min(1,max(cos_angle,-1)))

    def worker(self,input):
        print("Start Thread")
        data = self.model
        tri = data.points[input,:]
        axiswise1 = np.where(data == np.hstack((tri[0:3],tri[0:3],tri[0:3])), True, False)
        axiswise2 = np.where(data == np.hstack((tri[3:6],tri[3:6],tri[3:6])), True, False)
        axiswise3 = np.where(data == np.hstack((tri[6:9],tri[6:9],tri[6:9])), True, False)
        local_neighbors = []
        for j in range(0,axiswise1.shape[0]):
            count = 0
            if(np.all(axiswise1[j,0:3]) or np.all(axiswise1[j,3:6]) or np.all(axiswise1[j,6:9])):
                count+=1
            if(np.all(axiswise2[j,0:3]) or np.all(axiswise2[j,3:6]) or np.all(axiswise2[j,6:9])):
                count+=1
            if(np.all(axiswise3[j,0:3]) or np.all(axiswise3[j,3:6]) or np.all(axiswise3[j,6:9])):
                count+=1
            if(count == 2):
                #nearest_neighbors.append(i)#this triangle shares exactly 2 vertex with the original triangle
                local_neighbors.append([j, self.clean_acos(np.dot(data.normals[input], data.normals[j]) / (self.mag(data.normals[input]) * self.mag(data.normals[j])))])

        print("End Thread")
        return local_neighbors
    def run(self):
        start = time.time()
        pool = multiprocessing.Pool(processes=7)
        results = pool.map(self.worker,range(self.model.points.shape[0]))
        file = open('neighbors2.pkl','w')
        #pickle.dump(results,file)
        end = time.time()
        print(end - start)
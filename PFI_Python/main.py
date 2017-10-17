#from stl import mesh
import time
from pfitoolbox import ToolBox
import tasks
import matplotlib.pyplot as plt
import matplotlib
import trimesh
import numpy as np


from mpl_toolkits import mplot3d


name = 'chair'
name = 'Coffee_cup'

url = 'models/'+name+'.stl'
fileName = url.split('/')[-1]
print("Generating descriptor for usermodel: " + fileName)
try:
    model = trimesh.load_mesh(url)
except(OSError,IOError,ValueError):
    print("31: stl file missing")
    raise IOError

#ToolBox.faceDetector(model)
#ToolBox.localNeighborhoods(model)
facets = ToolBox.angle_splitter(model)
#facets.show()



#print(np.hstack((neighbors[:,0],np.dstack((neighbors[:,1],angles))[0])))

#print(pickle.load(open(name+'_neighbors.pkl','rb')))
#Start generating task chain
# id = "test"
# genDescriptorChain = []
# neighbors = ToolBox.loadNeighborsGraph(url)
# if(len(model.points) != len(neighbors)):#we don't have a neighbors entry fro every point so something is wrong, let's regenreate the neighbor's graph
#     indexes = range(0,len(model.points))
#     chunks= ToolBox.chunker(indexes,1000)#break the list into 10 parts
#     genGraphWorkflow = chord((findNeighborsTask.s(model,chunk) for chunk in chunks),reducer.s())
#     genDescriptorChain.append(genGraphWorkflow)
#
# genDescriptorChain.append(saveNeighbors.s(id))
#
# descriptorsChain = []
#
# descriptorsChain.append(angleHistTask.s())
# genDescriptorChain.append(chord(group(*descriptorsChain),reducer.s()))#make the descriptors a group so they can be executed in parallel, then use a chord to merge them
#
# genDescriptorChain.append(printResults.s())
# generate = chain(*genDescriptorChain)
#
# start = time.time()
# result = generate.delay()
#
# while (result.ready() == False):
#     print(time.time()-start)
#     time.sleep(1)
#
# print(result.get(timeout=1))


#point cloud
#figureno+=1
# figure = plt.figure(figureno)
# axes = mplot3d.Axes3D(figure)
# axes.scatter(model.points[:,0],model.points[:,1],model.points[:,2],color='b')

# #gridding
# figureno+=1
# figure = plt.figure(figureno)
# axes = mplot3d.Axes3D(figure)
# binsx = np.linspace(np.min(model.points[:,0]),np.max(model.points[:,0]),num=10)
# binsy = np.linspace(np.min(model.points[:,1]),np.max(model.points[:,1]),num=10)
# binsz = np.linspace(np.min(model.points[:,2]),np.max(model.points[:,2]),num=10)
# modelx = np.digitize(model.points[:,0], binsx)
# modely = np.digitize(model.points[:,1], binsy)
# modelz = np.digitize(model.points[:,2], binsz)
# axes.scatter(modelx,modely,modelz,color='b')

#first try at ordered drawing
#figureno+=1
# figure = plt.figure(figureno)
# axes = mplot3d.Axes3D(figure)
# co = [x / 100.0 for x in range(0, 100/model.points.shape[0], 100)]
# x = model.points[:,0]
# y = model.points[:,1]
# z = model.points[:,2]
# axes.scatter(x,y,z,color='k',cmap=cm.gist_rainbow)
# plt.show()

# #2D based gradient orientation
# saved_nearest_neighbors = pickle.load(open(name+'_neighbors.pkl','rb'))
# hog = []
# for i in range(0,model.points.shape[0]):
#     z = model.normals[i]/math.sqrt(model.normals[i,0]**2+model.normals[i,1]**2+model.normals[i,2]**2)
#     temp = np.array([z[0],z[2],z[1]])
#     x = temp-np.dot(temp,z)
#     y = np.cross(z,x)
#
#     p1 = [np.dot(model.points[i,0:3],x),np.dot(model.points[i,0:3],y),0]
#     p2 = [np.dot(model.points[i,3:6],x),np.dot(model.points[i,3:6],y),0]
#     p3 = [np.dot(model.points[i,6:9],x),np.dot(model.points[i,6:9],y),0]
#
#     m1 = np.true_divide((p2[1]-p1[1]),(p2[0]-p1[0]))
#     m2 = np.true_divide((p3[1]-p2[1]),(p3[0]-p2[0]))
#     m3 = np.true_divide((p3[1]-p1[1]),(p3[0]-p1[0]))
#
#     me = -(m1*m2+m1*m3+m2*m3+3)/(m1+m2+m3+3*m1*m2*m3);
#
#     if(math.isnan(me)):
#         me = 0
#
#     delta = np.array([saved_nearest_neighbors[i][0][1],saved_nearest_neighbors[i][1][1],saved_nearest_neighbors[i][2][1]])
#
#     index = np.argmax(delta)#get the index of the biggest delta
#     if(index == 0):
#         theta = 3.14-math.atan(1/m1)
#     elif(index == 1):
#         theta = -math.atan(1/m2)
#     elif(index == 2):
#         theta = 6.28-math.atan(1/m3)
#
#     magnitude = math.sqrt(delta[0]**2+delta[1]**2+delta[2]**2)
#     hog.append([magnitude,theta])
#
# nphog = np.array(hog)
#
# (hist, labels) = np.histogram(nphog[:,0],bins=20,density=True)
# labels = labels[:20]*180/3.14
# figureno += 1
# plt.figure(figureno)
# plt.bar(labels[0:20],hist,width=5)

#plotting point clouds filtered by gradient orientation
# centerx = (model.points[:,0]+model.points[:,3]+model.points[:,6])/3
# centery = (model.points[:,1]+model.points[:,4]+model.points[:,7])/3
# centerz = (model.points[:,2]+model.points[:,5]+model.points[:,8])/3
#
# filteredx = centerx[np.where(nphog[:,1]>1,True,False)]
# filteredy = centery[np.where(nphog[:,1]>1,True,False)]
# filteredz = centerz[np.where(nphog[:,1]>1,True,False)]
#
# figureno +=1
# figure = plt.figure(figureno)
# axes = mplot3d.Axes3D(figure)
# axes.set_xlim([axesLimits[0],axesLimits[1]])
# axes.set_ylim([axesLimits[0],axesLimits[1]])
# axes.set_zlim([axesLimits[0],axesLimits[1]])
# axes.scatter(filteredx,filteredy,filteredz)
#
# filteredx = centerx[np.where(nphog[:,1]<1,True,False)]
# filteredy = centery[np.where(nphog[:,1]<1,True,False)]
# filteredz = centerz[np.where(nphog[:,1]<1,True,False)]
#
# figureno +=1
# figure = plt.figure(figureno)
# axes = mplot3d.Axes3D(figure)
# axes.set_xlim([axesLimits[0],axesLimits[1]])
# axes.set_ylim([axesLimits[0],axesLimits[1]])
# axes.set_zlim([axesLimits[0],axesLimits[1]])
# axes.scatter(filteredx,filteredy,filteredz)


# gradient orientation spherical coordinates

#
# saved_nearest_neighbors = pickle.load(open(name+'_neighbors.pkl','rb'))
#
# start = time.clock()
#
# gradOrientations = []
# for i in range(0,len(saved_nearest_neighbors)):
#     x = saved_nearest_neighbors[i][0][1]
#     y = saved_nearest_neighbors[i][1][1]
#     z = saved_nearest_neighbors[i][2][1]
#     r = math.sqrt(x**2+y**2+z**2)
#     theta = math.acos(z/r)
#     phi = math.atan2(x,y)
#     gradOrientations.append([theta,phi,r])
#
# print(time.clock() - start, "seconds process time")
# gradOrientations = np.array(gradOrientations)
# (hist, labels) = np.histogram(gradOrientations[:,0],weights=gradOrientations[:,2],bins=9,density=True)
# labels = labels[:9]*180/3.14
# figureno += 1
# plt.figure(figureno)
# plt.bar(labels[0:9],hist,width=9)
# plt.title("theta")
#
# (hist, labels) = np.histogram(gradOrientations[:,1],weights=gradOrientations[:,2],bins=9,density=True)
# labels = labels[:9]*180/3.14
# figureno += 1
# plt.figure(figureno)
# plt.bar(labels[0:9],hist,width=9)
# plt.title("phi")
#
# (hist, labels) = np.histogram(gradOrientations[:,2],bins=9,density=True)
# figureno += 1
# plt.figure(figureno)
# plt.bar(labels[0:9],hist,width=0.4)
# plt.title("r")

#find neighborhoods

# binsx = np.linspace(np.min(model.points[:,0]),np.max(model.points[:,0]),num=20)
# binsy = np.linspace(np.min(model.points[:,1]),np.max(model.points[:,1]),num=20)
# binsz = np.linspace(np.min(model.points[:,2]),np.max(model.points[:,2]),num=20)
# modelx = np.digitize(model.points[:,0], binsx)
# modely = np.digitize(model.points[:,1], binsy)
# modelz = np.digitize(model.points[:,2], binsz)
#
# griddedModel = np.vstack((np.vstack((modelx,modely)),modelz))
#
# neighborhoods = np.sum(griddedModel,0)
# print(neighborhoods.shape)
#
#
# (hist2, labels) = np.histogram(neighborhoods,bins=8000,density=True)
# neighborhoodsLabels = np.digitize(neighborhoods,labels)
#
# figureno +=1
# plt.figure(figureno)
# axes = mplot3d.Axes3D(figure)
# axes.scatter(model.points[:,0],model.points[:,1],model.points[:,2],c=neighborhoodsLabels,cmap=cm.gist_rainbow)

# (hist2, labels) = np.histogram(angles,bins=20,density=True)
# #labels = np.arange(1,len(hist)+1)
# labels = labels[:20]*180/3.14
#figureno +=1
# plt.figure(figureno)
# plt.bar(labels[0:20],hist2,width=5)

#---------------
# dMap = np.zeros((model.points.shape[0],model.points.shape[0]))
# start = time.time()
# for i in range(0,model.points.shape[0]):
#     print(i)
#     for j in range(0,model.points.shape[0]):
#         dMap[i,j]= clean_acos(np.dot(model.normals[i], model.normals[j]) / (mag(model.normals[i]) * mag(model.normals[j])))
#
# np.save(name+'_dmap.npy',dMap)
# end = time.time()
# print(end - start)

# try:
#     dMap = np.load(name+'_dmap.npy')
# except IOError:
#     dMap = np.zeros((model.points.shape[0],model.points.shape[0]))
#     start = time.time()
#     for i in range(0,model.points.shape[0]):
#         print(i)
#         for j in range(0,model.points.shape[0]):
#             dMap[i,j]= clean_acos(np.dot(model.normals[i], model.normals[j]) / (mag(model.normals[i]) * mag(model.normals[j])))
#
#     np.save(name+'_dmap.npy',dMap)
#     end = time.time()
#     print(end - start)
# figureno+=1
# plt.figure(figureno)
# plt.imshow(dMap, extent=(0, 120, 120, 0),
# interpolation='nearest', cmap=cm.Greys)

# dMap2 = np.load('hex2_dMap.npy')
# figureno+=1
# plt.figure(figureno)
# plt.imshow(dMap2, extent=(0, 120, 120, 0),
# interpolation='nearest', cmap=cm.Greys)

# #--------------
#plt.show()

#distance map
# t0 = time.clock()
# dists = scipy.spatial.distance.pdist(model.points[:,0:3],'euclidean')
# dists = scipy.spatial.distance.squareform(dists)
# print(time.clock() - t0, "seconds process time")
# plt.imshow(dists,interpolation='nearest', cmap=cm.gist_rainbow)


# figureno += 1
# figure = plt.figure(figureno)
# dists2 = scipy.spatial.distance.pdist(model2.points[:,0:3],'euclidean')
# dists2 = scipy.spatial.distance.squareform(dists2)
# plt.imshow(dists2, extent=(0, 1.6, 1.6, 0),
#          interpolation='nearest', cmap=cm.gist_rainbow)
# figureno += 1
# figure = plt.figure(figureno)
# diff = dists-dists2[0:5438,0:5438]
# plt.imshow(diff, extent=(0, 1.6, 1.6, 0),
#            interpolation='nearest', cmap=cm.gist_rainbow)
#


# surface_distance = np.zeros((model.normals.shape[0],model.normals.shape[0]))
# saved_nearest_neighbors = pickle.load(open(name+'_neighbors.pkl','rb'))
# dists = scipy.spatial.distance.pdist(model.points[:,0:3],'euclidean')
# dists = scipy.spatial.distance.squareform(dists)
# print(dists.shape)
# for facet in saved_nearest_neighbors:
#     for neighbor in facet:
#         surface_distance[facet[0][0],neighbor[0]] = dists[facet[0][0],neighbor[0]];
#
# surf_dists = scipy.sparse.csgraph.dijkstra(surface_distance,directed=False)
# figureno+=1
# plt.figure(figureno)
# plt.imshow(surf_dists,interpolation='nearest', cmap=cm.gist_rainbow)


# plt.show()


# nearest_neighbors = []
#

#
# for i in range(0,model.points.shape[0]):
#     tri = model.points[i,:]
#     axiswise1 = np.where(model.points == np.hstack((tri[0:3],tri[0:3],tri[0:3])), True, False)
#     axiswise2 = np.where(model.points == np.hstack((tri[3:6],tri[3:6],tri[3:6])), True, False)
#     axiswise3 = np.where(model.points == np.hstack((tri[6:9],tri[6:9],tri[6:9])), True, False)
#     local_neighbors = []
#     for j in range(0,axiswise1.shape[0]):
#         count = 0
#         if(np.all(axiswise1[j,0:3]) or np.all(axiswise1[j,3:6]) or np.all(axiswise1[j,6:9])):
#             count+=1
#         if(np.all(axiswise2[j,0:3]) or np.all(axiswise2[j,3:6]) or np.all(axiswise2[j,6:9])):
#             count+=1
#         if(np.all(axiswise3[j,0:3]) or np.all(axiswise3[j,3:6]) or np.all(axiswise3[j,6:9])):
#             count+=1
#         # print(tri)
#         # print(model.points[j,:])
#         # print(count)
#         if(count == 2):
#             #nearest_neighbors.append(i)#this triangle shares exactly 2 vertex with the original triangle
#             local_neighbors.append([j, clean_acos(np.dot(model.normals[i], model.normals[j]) / (mag(model.normals[i]) * mag(model.normals[j])))])

#
#     nearest_neighbors.append([i, local_neighbors])
#
# file = open('neighbors.pkl','w')
# pickle.dump(nearest_neighbors,file)

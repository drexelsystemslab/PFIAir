import scipy
from scipy import misc

def construct(v,f):
    vs = []
    fs = []

    for index, v in enumerate(v):
        vs.append('v  ' + str(v[0]/float(100)) + ' ' + str(v[1]/float(100)) + ' ' + str(v[2]/float(100)))
        #print('v  ' + str(v[0]) + ' ' + str(v[1]) + ' ' + str(v[2]))

    # adding faces
    for index, face in enumerate(f):
        fs.append('f  ' + str(face[0] + 1) + ' ' + str(face[1] + 1) + ' ' + str(face[2] + 1))


    cat = vs + fs
    out_file = '\n'.join(cat)
    #print(out_file)

    #print out_file
    nf = open('./' + "test_small" + '.obj','w')
    nf.write(out_file)
    nf.close()

if __name__ == "__main__":

    vss = []
    fss = []

    img = scipy.misc.imread('depth_small.jpg', flatten=False, mode='L')

    height, width = img.shape
    for y in range(0, height - 1):
        for x in range(0, width - 1):
            fss.append([x + y*width,  x + y*width + 1, x + (y+1)*width + 1])
            fss.append([x + (y+1)*width, x + y*width, x + 1 + (y+1)*width])

    #print(fs[0:10])

    for y,row in enumerate(img):
        for x,col in enumerate(row):
            vss.append([x,y,col])

    # print(vss)
    # print(fss)

    construct(vss,fss)

def extracrSeg(file_name,seg_num,part_num):
    f = open('./' + file_name + '.off').read().split('\n')
    assert (f[0] == 'OFF')

    seg = open('./' + file_name + '_' + str(seg_num) + '.seg').read().split('\n')[:-1]

    vertices, faces, total = f[1].split(' ')
    vertices = int(vertices)
    faces = int(faces)
    assert (len(seg) == faces)


    vs = f[2:vertices+2]
    fs = f[vertices+2:-1]

    for index, v in enumerate(vs):
        vs[index] = 'v  ' + v

    # adding faces
    for index, face in enumerate(fs):
        if not (int(seg[index]) == part_num):
            continue
        a,b,c = face[2:].split(' ')
        a = int(a)
        b = int(b)
        c = int(c)
        fs[index] = 'f  ' + str(a + 1) + ' ' + str(b + 1) + ' ' + str(c + 1)


    cat = vs + fs
    out_file = '\n'.join(cat)

    #print out_file
    nf = open('./' + file_name + '_' + str(seg_num) + '_' + str(part_num) + '.obj','w')
    nf.write(out_file)
    nf.close()



extracrSeg('24',0,0)


#include "jmesh.h"
#include "fittingPrimitives.h"
#include <string>
#include <iostream>

using namespace std;

int main()
{
 JMesh::init();
 Triangulation *nt = new Triangulation;
 char s[] = "cube.stl";
 char name[] = "cube";
 int ioerr = nt->load(s);
 if (ioerr == IO_CANTOPEN) JMesh::warning("Couldn't open '%s'\n",s);
 else if (ioerr == IO_UNKNOWN) JMesh::warning("'%s': Unknown file format.\n",s);
 else if (nt->shells() == 0)
 {
  JMesh::warning("No triangles loaded!\n");
  delete nt;
 }

 unsigned char wtf = 0;
 wtf |= HFP_FIT_PLANES;
 wtf |= HFP_FIT_SPHERES;
 wtf |= HFP_FIT_CYLINDERS;

 HFP_Action hfp(nt, 5);
 List* collapses = hfp.fit(wtf,name);
//
// Node* n;
// int counter = 0;
// for ((n) = collapses->head(); (n) != NULL; (n)=(n)->next()){
//     if(counter%1)
//        cout << n->data << "\n";
// }

  //Triangle *f;
 //Node *n;
 //int i=0; FOREACHVTTRIANGLE((&(tin->T)), f, n) f->info = (void *)i++;
 //g++ -o segmentation -L/home/bahareh/Documents/JMeshLib-1.2/lib -I/home/bahareh/Documents/JMeshLib-1.2/include segmentation.cpp fittingPrimitives.cpp -ljmesh


}

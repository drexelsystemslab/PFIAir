import sys
from pfitoolbox import ToolBox
import trimesh
import networkx as nx


if __name__ == "__main__":
    if(len(sys.argv) != 4):
        print("Usage: python main.py <input oof> <output oof> <segment list file>")
    else:
        try:
            model = trimesh.load_mesh(sys.argv[1])
        except(OSError, IOError, ValueError):
            print("stl file missing")
            raise IOError

        filename = sys.argv[1].split(".",2)[0]

        try:
            contraction_graph = nx.read_gpickle(filename+".gpickle")
        except IOError:
            print("generating contraction graph for ",filename)
            contraction_graph = ToolBox.faceClustering(model)

        nx.write_gpickle(contraction_graph,filename+".gpickle")
        # with open(filename+".hst","w") as file:

        seg = ToolBox.contraction_graph_to_seg(contraction_graph, model)

        with open(sys.argv[3],"w") as file:
            for face in seg:
                file.write(face)

        with open(sys.argv[2],"w") as file:#go ahead and write the model back out to the output in case something needs it
            model.export(file,"off")




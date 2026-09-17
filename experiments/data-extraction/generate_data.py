import math
import os
import paraview
import tempfile
import vtk

from pathlib import Path
from restrictions import get_ttk_objects, simple_restrictions, MergeTree
from paraview.simple import *

paraview.compatibility.major = 6
paraview.compatibility.minor = 0

SCALAR_FIELD_NAME = "scalars-split"
SIMPLIFICATION_THRESHOLD = 0.0

DATA_GENERATION_DIRECTORY = "data/generated"

# #### disable automatic camera reset on 'Show'
# paraview.simple._DisableFirstRenderCameraReset()

# def red_sea_timesteps(directory, begin, end):
#     for i in range(begin, end):
#         data_0vtp = XMLPolyDataReader(registrationName='data_0.vtp', FileName=[f'{directory}/data_{i}.vtp'])
#         data_0vtp.Set(
#             PointArrayStatus=['Normals', 'TextureCoordinates', 'Scalars_'],
#             TimeArray='None',
#         )
#         calculator1 = Calculator(registrationName='Calculator1', Input=data_0vtp)
#         calculator1.Set(
#             ResultArrayName='scalars',
#             Function='Scalars_',
#         )
#         calculator2 = Calculator(registrationName='Calculator2', Input=calculator1)
#         calculator2.Set(
#             ResultArrayName=SCALAR_FIELD_NAME,
#             Function='-scalars',
#         )
#         SetActiveSource(calculator2)
#         SaveData(
#             f"{directory}/step_{i:02d}.vtp",
#             proxy=calculator2,
#             PointDataArrays=["scalars", SCALAR_FIELD_NAME]
#         )



def generate_timesteps(filename):
    reader = vtk.vtkXMLImageDataReader()
    reader.SetFileName(filename)
    reader.update()

    image = reader.GetOutput()
    extent = image.GetExtent()
    z_min, z_max = extent[4], extent[5]

    for z in range(z_min, z_max + 1):
        slice = vtk.vtkExtractVOI()
        slice.SetInputData(image)
        slice.SetVOI(extent[0], extent[1], extent[2], extent[3], z, z)
        slice.update()
    
        writer = vtk.vtkXMLImageDataWriter()
        writer.SetInputData(slice.GetOutput())
        writer.SetFileName(os.path.join(tmp_directory, f"step_{z:04d}.vti"))
        writer.Write()

def output_merge_tree(f, tree : MergeTree):
    out = []
    num_leaves = len(tree.leaves)
    out.append(f"{num_leaves}")
    
    for leaf in tree.leaves:
        id = tree.id_map[leaf]
        height = tree.get_height(leaf)
        x, y = tree.get_coords(leaf)
        out.append((f"{id} {height} {x} {y} {leaf}"))
    
    num_merges = len(tree.merges)
    num_merges_id = len(out)
    count = 0
    
    removed = {}
    
    out.append(f"{num_merges}")
    new_nodes = []
    for merge in tree.merges:   
        id = tree.id_map[merge] + len(new_nodes)
        height = tree.get_height(merge)
        x, y = tree.get_coords(merge)
        
        children_with_removed = [tree.id_map[child_index] + len([i for i in new_nodes if i <= tree.id_map[child_index]]) for child_index in tree.children[merge]]
        
        children = []
        for child in children_with_removed:
            if child in removed:
                child = removed[child]
            children.append(child)
        
        if len(children) == 1:
            removed[id] = children[0]
        elif len(children) == 2:
            out.append(f"{id} {height} {x} {y} {' '.join(map(str, children))} {merge}")
            count += 1
        elif len(children) == 3:
            new_height = math.nextafter(height, -math.inf)
            new_nodes.append(id)
            out.append(f"{id} {new_height} {x} {y} {' '.join(map(str, children[1:]))} {merge}")
            out.append(f"{id+1} {height} {x} {y} {' '.join(map(str, [children[0], id]))} {merge}")
            count += 2
        else:
            print(merge, children)
            assert False, "more than 3 children!"
    out[num_merges_id] = count
    f.write('\n'.join(map(str, out)))
    f.write('\n')

def generate_instances(radii, min_step, max_step, step, name):
    output_directory = os.path.join(DATA_GENERATION_DIRECTORY, f'{name}')
    os.makedirs(output_directory, exist_ok=True)

    terrains = {}
    merge_trees = {}
    
    files = os.listdir(tmp_directory)
    
    for i in range(min_step, min(len(files), max_step+1), step):
        file_path = os.path.join(tmp_directory, files[i])
        terrains[i], merge_trees[i] = get_ttk_objects(file_path, SCALAR_FIELD_NAME, SIMPLIFICATION_THRESHOLD)
        output_path = os.path.join(output_directory, f'merge-tree-{i:04d}.txt')
        with open(output_path, "w") as f:
            output_merge_tree(f, merge_trees[i])

    for radius in radii:
        restrictions_directory = os.path.join(output_directory, f'radius-{"∞" if radius == math.inf else radius}')
        os.makedirs(restrictions_directory, exist_ok=True)
        for i in range(min_step, min(len(files), max_step+1), step):
            for j in range(i, min(len(files), max_step+1), step):
                restriction = simple_restrictions(terrains[i], terrains[j], merge_trees[i], merge_trees[j], radius)

                output_path = os.path.join(restrictions_directory, f'{i:04d}_{j:04d}.txt')
                with open(output_path, "w") as f:
                    f.write("\n".join([" ".join(map(str, row)) for row in restriction]))

def heatedcylinder():
    stack = "data/raw/heatedcylinder.vti"
    generate_timesteps(stack)

    radii = [0, 10, math.inf]
    begin = 1000
    end = 1500
    step = 50
    name = "heated-cylinder"

    generate_instances(radii, begin, end, step, name)

def redsea():
    stack = "data/raw/redsea"
    radii = [0, 10, 25, math.inf]
    begin = 0
    end = 59

    red_sea_timesteps(begin, end)

    step = 5
    name = "redsea"
    
    generate_instances(radii, begin, end, step, name)

if __name__ == "__main__":

    # data_directory = "data/raw"
    # with tempfile.TemporaryDirectory() as tmp_directory:
    #     tmp_directory = Path(tmp_directory)

    tmp_directory = "data/tmp"
    heatedcylinder()

    # with tempfile.TemporaryDirectory() as tmp_directory:
    #     tmp_directory = Path(tmp_directory)
    #     generate_timesteps("data/raw/redsea.vtp")
    #     redsea()
import argparse
import math
import vtk

from paraview.simple import servermanager, Tetrahedralize, TTKMergeTree, TTKTopologicalSimplificationByPersistence, XMLImageDataReader,XMLPolyDataReader

LEAF_TYPE = 0
MERGE_TYPE = 1

class Terrain:
    
    def __init__(self, ttk_terrain, scalar_field_name, dimensions):
        self.terrain_data = servermanager.Fetch(ttk_terrain)
        self.heights = self.terrain_data.GetPointData().GetArray(scalar_field_name)
        self.dimensions = dimensions
    
    def get_height(self, vertex):
        return self.heights.GetValue(vertex)
    
    def get_coords(self, vertex):
        x, y, _ = self.terrain_data.GetPoint(vertex)
        return x, y
    
    def get_disc(self, vertex, radius):
        nx, ny, _  = self.dimensions
        vx = vertex % nx
        vy = vertex // nx
        
        disc = []
        for x in range(max(0, vx-radius), min(nx, vx+radius+1)):
            for y in range(max(0, vy-radius), min(ny, vy+radius+1)):
                if (x - vx)**2 + (y - vy)**2 <= radius**2:
                    disc.append(nx * y + x)
        return disc
    
    def radius_larger_than_terrain(self, radius):
        nx, ny, _ = self.dimensions
        return radius > nx and radius > ny

class MergeTree:
    
    def __init__(self, ttk_merge_tree, terrain : Terrain):
        self.nodes = servermanager.Fetch(ttk_merge_tree, idx=0).GetPointData()        
        self.edges = servermanager.Fetch(ttk_merge_tree, idx=1).GetCellData()
        self.segments = servermanager.Fetch(ttk_merge_tree, idx=2).GetPointData()
        
        self.terrain = terrain
        
        self.children, self.parents = self.initialize_merge_tree()
        self.leaves, self.merges, self.id_map = self.compute_indexing()
    
    def get_vertex(self, node):
        return self.nodes.GetArray("VertexId").GetValue(node)
        
    def get_height(self, node):
        vertex = self.get_vertex(node)
        return self.terrain.get_height(vertex)
    
    def get_coords(self, node):
        vertex = self.get_vertex(node)
        return self.terrain.get_coords(vertex)
    
    def get_critical_type(self, node):
        return self.nodes.GetArray("CriticalType").GetValue(node)
    
    def get_up_node(self, edge):
        return self.edges.GetArray("upNodeId").GetValue(edge)
    
    def get_down_node(self, edge):
        return self.edges.GetArray("downNodeId").GetValue(edge)
    
    def get_target_node(self, vertex):
        segment = self.segments.GetArray("SegmentationId").GetValue(vertex)
        edge_to_segment_id = self.edges.GetArray("SegmentationId")
        for i in range(self.edges.GetNumberOfTuples()):
            if segment == edge_to_segment_id.GetValue(i):
                return self.get_down_node(i)
        
    def initialize_merge_tree(self):
        num_nodes = self.nodes.GetNumberOfTuples()
        num_edges = self.edges.GetNumberOfTuples()
        assert num_edges == num_nodes-1, f"Number of edges is not equal to number of nodes - 1"

        children = {i : [] for i in range(num_nodes)}
        parents = {i: None for i in range(num_nodes)}
        
        for edge in range(num_edges):
            children[self.get_up_node(edge)].append(self.get_down_node(edge))
            parents[self.get_down_node(edge)] = self.get_up_node(edge)
            
        for node, node_children in children.items():
            if len(node_children) > 2:
                print(node, node_children)
        
        return children, parents

    def compute_indexing(self):
        num_nodes = self.nodes.GetNumberOfTuples()
        
        ## Map from original node indices to new indexing (first all leaves, then internal nodes)
        leaves = [node for node in range(num_nodes) if self.get_critical_type(node) == LEAF_TYPE]
        merges = [node for node in range(num_nodes) if self.get_critical_type(node) != LEAF_TYPE]
        id_map = {}
        for i, leaf in enumerate(leaves):
            id_map[leaf] = i
        for i, merge in enumerate(merges):
            id_map[merge] = len(leaves) + i
    
        return leaves, merges, id_map

def get_ttk_objects(file_name, scalar_field_name, threshold):
    
    if file_name.endswith('.vti'):
        terrain = XMLImageDataReader(FileName=[file_name])
    elif file_name.endswith('.vtp'):
        terrain = XMLPolyDataReader(FileName=[file_name])
    
    triangulation = Tetrahedralize(Input=terrain)
    print("triangulation: ", triangulation)
    simplified_terrain = TTKTopologicalSimplificationByPersistence(Input=triangulation)
    simplified_terrain.Set(
        InputArray=['POINTS', scalar_field_name],
        PersistenceThreshold=threshold,
        ThresholdIsAbsolute=1,
    )
    
    if file_name.endswith('.vti'):
        reader = vtk.vtkXMLImageDataReader()
        reader.SetFileName(file_name)
        reader.Update()
        terrainObject = Terrain(simplified_terrain, scalar_field_name, reader.GetOutput().GetDimensions())   
    else:
        terrainObject = Terrain(simplified_terrain, scalar_field_name, [150, 100, 1]) # hardcoded dimension for red sea data set..
    
    merge_tree = TTKMergeTree(
        Input=simplified_terrain,
        ScalarField = scalar_field_name,
        TreeType="Join Tree"
    )
    
    return terrainObject, MergeTree(merge_tree, terrainObject)
    
def simple_restrictions(source_terrain : Terrain, target_terrain : Terrain, source_tree : MergeTree, target_tree : MergeTree, radius):
    matrix = [[None for _ in target_tree.leaves] for _ in source_tree.leaves]
    
    if source_terrain.radius_larger_than_terrain(radius):
        return [[target_tree.get_height(i) for i in target_tree.leaves] for _ in source_tree.leaves]
        
    for v in source_tree.leaves:
        height_v = source_tree.get_height(v)
        
        vertex_of_v = source_tree.get_vertex(v)
        # print(f'vertex id: {vertex_of_v}')
        assert source_terrain.get_height(vertex_of_v) == height_v, f"leaf height {height_v} ≠ vertex height {source_terrain.get_height(vertex_of_v)}"
        
        # Height of v projected in the target terrain
        # height_v_in_target = target_terrain.get_height(vertex_of_v)

        search_disc = source_terrain.get_disc(vertex_of_v, radius)
        
        ## Find the contour components that contain the points in the search disc
        target_nodes = set([target_tree.get_target_node(v) for v in search_disc])
        # print(target_nodes)
        ancestors = target_nodes.copy()
        for target_node in target_nodes:
            current_node = target_tree.parents[target_node]
            while current_node != None and current_node not in ancestors:
                ancestors.add(current_node)
                current_node = target_tree.parents[current_node]
                
        # print(ancestors)
        for w in target_tree.leaves:
            # find lca of target node and target leaf
            lca = w
            while lca not in ancestors:
                assert target_tree.parents[lca] != None, "there is no lca of target node and target leaf"
                lca = target_tree.parents[lca]
            
            height_lca = target_tree.get_height(lca)
            i = source_tree.id_map[v]
            j = target_tree.id_map[w]
            matrix[i][j] = max(height_v, height_lca)
    
    return matrix

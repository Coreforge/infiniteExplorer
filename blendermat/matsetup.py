import bpy
import json
import os.path

# debug globals
dbg_shaderdb = "/media/bigDOwO/halo/infinite/bsptest/shaderdb.json"
dbg_matdb = "/media/bigDOwO/halo/infinite/bsptest/test.dae.json"
dbg_texpath = "/media/bigDOwO/halo/infinite/bsptest/tex/"
dbg_shaderpath = "/media/bigDOwO/halo/infinite/bsptest/"
# testing with /sbsp/0183a942

class MaterialImporterContext:
    def __init__(self) -> None:
        self.texpath = ""

    # this currently doesn't fully support texture arrays, the first frame will always be used
    def getTexPath(self, id : str):
        return f"{self.texpath}/{id}.png"
    
    def getImage(self, id : str):
        if id in bpy.data.images.keys():
            return bpy.data.images[id]
        
        if not os.path.isfile(self.getTexPath(id)):
            # texture doesn't exist
            if f"{id}_dummy" in bpy.data.images.keys():
                return bpy.data.images[f"{id}_dummy"]
            img = bpy.data.images.new(f"{id}_dummy", 1, 1)
            return img
        img = bpy.data.images.load(self.getTexPath(id))
        img.name = id
        return img
    
    def loadShader(self, name : str, filename : str):
        with bpy.data.libraries.load(f"{dbg_shaderpath}/{filename}", link=False) as (data_from, data_to):
            data_to.materials = [name]

    def getUVName(self, idx : int):
        return f"UV{idx}"


# copies nodes, default values of sockets, and links from one material to another one
# existing nodes in the dstMat get deleted
# there's a bit of a limitation here: a node must not have multiple inputs or outputs with the same name, or they can't be linked correctly
# use group nodes to get around this
def copyNodes(srcMat : bpy.types.Material, dstMat : bpy.types.Material):
    # clear dst material
    for node in dstMat.node_tree.nodes:
        dstMat.node_tree.nodes.remove(node)
    
    for node in srcMat.node_tree.nodes:
        new_node = dstMat.node_tree.nodes.new(node.bl_idname)
        new_node.label = node.label
        new_node.location = node.location
        new_node.name = node.name

        if(node.bl_idname == 'ShaderNodeGroup'):
            new_node.node_tree = node.node_tree

        if(node.bl_idname == 'ShaderNodeMath'):
            new_node.operation = node.operation
            new_node.use_clamp = node.use_clamp
        
        new_node.color = node.color
        new_node.width = node.width
        new_node.height = node.height
        for input in node.inputs:
            if input.type == "SHADER":
                # there's nothing I can set this to I think
                continue
            new_node.inputs[input.name].default_value = input.default_value
        new_node.update()
    # set up links/relationships between nodes after all nodes have been copied
    for node in srcMat.node_tree.nodes:
        new_node = dstMat.node_tree.nodes[node.name]
        if node.parent is not None:
            new_node.parent = dstMat.node_tree.nodes[node.parent.name]
    
    for link in srcMat.node_tree.links:
        #print(f"Link from {link.from_node.name}:{link.from_socket.identifier} to {link.to_node.name}:{link.to_socket.identifier}")
        #print(dstMat.node_tree.nodes[link.to_node.name].inputs.values())
        dstMat.node_tree.links.new(dstMat.node_tree.nodes[link.from_node.name].outputs[link.from_socket.name],
                                   dstMat.node_tree.nodes[link.to_node.name].inputs[link.to_socket.name])


# returns the shader description if a shader is found, or None otherwise
def getBestMat(materialName : str ,materialParameters : dict, shaderdb : dict):
    # there's probably more efficient methods, but this should be simpler to implement
    # results could also be cached and tweaked
    matches = []
    matches.extend((0,) * len(shaderdb["shaders"]))
    for i,shaderdesc in enumerate(shaderdb["shaders"]):
        if "mat_blacklist" in shaderdesc.keys():
            if materialName in shaderdesc["mat_blacklist"]:
                matches[i] = -1 # blacklisted
                continue
        if not "parameters" in shaderdesc.keys():
            matches[i] = -1 # no parameters -> not useable
            continue
        for param in shaderdesc["parameters"]:
            if param["name"] in materialParameters.keys():
                matches[i]+=1   # the material has this parameter
            elif not ("optional" in param.keys() and param["optional"] is True):
                matches[i]-=2   # the material doesn't have this parameter (maybe also just discard it completely as an option instead)

    currentmax = -1
    idx = -1
    for i, score in enumerate(matches):
        # maybe add priority in case multiple shaders match the same number of parameters. Or just improve the whole thing, this is essentially just for a quick test
        if score > currentmax: 
            currentmax = score
            idx = i
    if currentmax > 0:
        winningshader = shaderdb["shaders"][idx]
        print(f"Found matching shader {winningshader['name']} for material {materialName} with score {currentmax}")
        return winningshader
    print(f"Could not find a fitting shader for material {materialName}")
    return None


def setupMatParameters(mat : bpy.types.Material, shaderdesc : dict, materialdesc : dict, context : MaterialImporterContext):
    for parameter in shaderdesc["parameters"]:
        #print(parameter)
        if not parameter["maps_to_node"] in mat.node_tree.nodes.keys():
            print(f"Error: Could not find node {parameter['maps_to_node']} in material {mat.name}")
            continue
        if parameter["name"] not in materialdesc.keys():
            print(f"Parameter {parameter['name']} not in material {mat.name}")
            continue
        if "ignore" in parameter.keys() and parameter["ignore"] == True:
            continue

        node = mat.node_tree.nodes[parameter["maps_to_node"]]
        if parameter["type"] == "texture":
            node.image = context.getImage(materialdesc[parameter["name"]]["name"])

        elif parameter["type"] == "uv":
            node.uv_map = context.getUVName(materialdesc[parameter["name"]])
        elif parameter["type"] == "uv3":    # special type for the UV3 parameter that sets the UV index to 2
            if materialdesc[parameter["name"]] == True:
                node.uv_map = context.getUVName(2)

        elif parameter["type"] == "float":
            if not parameter["input"] in node.inputs.keys():
                print(f"Error: Could not find input socket {parameter['input']} on node {parameter['maps_to_node']}")
                continue
            node.inputs[parameter["input"]].default_value = materialdesc[parameter["name"]]

        elif parameter["type"] == "condition_float":
            if not parameter["input"] in node.inputs.keys():
                print(f"Error: Could not find input socket {parameter['input']} on node {parameter['maps_to_node']}")
                continue
            node.inputs[parameter["input"]].default_value = parameter["value"]
        else:
            print(f"Unsupported parameter type {parameter['type']}")

def setupMat(mat : bpy.types.Material, shaderdesc : dict, materialdesc : dict, context : MaterialImporterContext):
    if shaderdesc["name"] not in bpy.data.materials.keys():
        context.loadShader(shaderdesc["name"], shaderdesc["file"])
        if shaderdesc["name"] not in bpy.data.materials.keys():
            # still not present/couldn't load
            print("Material not present in file (I still need to add loading materials from other blends)")
            return
    
    copyNodes(bpy.data.materials[shaderdesc["name"]], mat)
    setupMatParameters(mat, shaderdesc, materialdesc, context)

# rename UVs to uniform names across all meshes using materials in materialdb
def renameUVs(materialdb : dict, context : MaterialImporterContext):
    for mesh in bpy.data.meshes.values():
        usesMat = False
        #print(f"mesh: {mesh.name}")
        for mat in mesh.materials.keys():
            #print(f"Mat: {mat}")
            if mat in materialdb.keys():
                usesMat = True
        # this is just a simple filter to prevent messing up other meshes (doesn't work properly)
        if True:
            for i,uv in enumerate(mesh.uv_layers):
                uv.name = context.getUVName(i)


def processMat(name : str, context : MaterialImporterContext):
    shaderdesc = getBestMat(name, matdb[name], shaderdb)
    if shaderdesc is not None:
        setupMat(bpy.data.materials[name], shaderdesc, matdb[name], importcontext)

if __name__ == "__main__":
    # test stuff

    importcontext = MaterialImporterContext()
    importcontext.texpath = dbg_texpath

    shaderdb_f = open(dbg_shaderdb,'rb')
    shaderdb = json.load(shaderdb_f)
    shaderdb_f.close()

    matdb_f = open(dbg_matdb,'rb')
    matdb = json.load(matdb_f)
    matdb_f.close()

    #tmat = getBestMat("/mat /3f72d37b", matdb["/mat /3f72d37b"], shaderdb)
    #print(tmat)
    #copyNodes(bpy.data.materials["testmat"], bpy.data.materials["/mat /3f72d37b"])
    #setupMat(bpy.data.materials["/mat /3f72d37b"], tmat, matdb["/mat /3f72d37b"], importcontext)
    #processMat("/mat /3f72d37b", importcontext)
    #processMat("/mat /8096811d", importcontext)
    #processMat("/mat /8779b860", importcontext)
    
    # set up all materials (or try to) from the file
    # this might take a while, so probably don't use that when just testing
    renameUVs(matdb, importcontext)
    
    for mat in matdb.keys():
        if mat in bpy.data.materials.keys():
            #print(f"Material: {mat}")
            processMat(mat, importcontext)
import os
import struct
import tqdm
import open3d as o3d
import pymeshlab

# 遍历目录及子目录下的所有fbx文件
def get_files(path):
    ffiles = []
    for root, _, files in os.walk(path):
        for file in files:
            if file.lower().endswith('.fbx'):
                ffiles.append(os.path.join(root, file)[len(path) + 1:])
    return ffiles

FBXDIR = r'C:\Program Files\tadsim\resources\app\dist\electron\assets\models'
DATDIR = 'models'

files = get_files(FBXDIR)
faild = []
for fbxfile in tqdm.tqdm(files):
    # if os.path.exists(os.path.join(DATDIR, fbxfile.replace('.fbx', '.txt'))):
    #    continue
    objfile = fbxfile.replace('.fbx', '.obj')
    # mesh = o3d.io.read_triangle_mesh(os.path.join(FBXDIR, fbxfile))
    # if not mesh.has_triangles():
    # if not os.path.exists(os.path.join(FBXDIR, objfile)):               
    ms = pymeshlab.MeshSet()
    ms.load_new_mesh(os.path.join(FBXDIR, fbxfile))
    ms.load_filter_script('Pure-TriangularMesh.mlx')
    ms.apply_filter_script()
    ms.save_current_mesh(os.path.join(FBXDIR, objfile), save_textures=False, save_vertex_coord=False, 
                         save_vertex_normal=True, save_polygonal=False, save_wedge_texcoord=False)
    mesh = o3d.io.read_triangle_mesh(os.path.join(FBXDIR, objfile))
    if not mesh.has_triangles():
        faild.append(fbxfile)
        continue
    pcd = mesh.sample_points_uniformly(number_of_points=1024)
    os.makedirs(os.path.dirname(os.path.join(DATDIR, fbxfile)), exist_ok=True)
    with open(os.path.join(DATDIR, fbxfile+'.smp'), 'wb') as f:
        for p in pcd.points:
            f.write(struct.pack('<f', p[0]))
            f.write(struct.pack('<f', p[1]))
            f.write(struct.pack('<f', p[2]))
    with open(os.path.join(DATDIR, fbxfile+'.txt'), 'w') as f:
        for p in pcd.points:
            f.write(str(p[0]) + ' ' + str(p[1]) + ' ' + str(p[2]) + '\n')

print('Error: ', faild)

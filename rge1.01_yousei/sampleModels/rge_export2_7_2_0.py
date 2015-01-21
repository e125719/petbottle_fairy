# rge_export2.6.3.py (c) 2010-2012, Yuhei Akamine
#
# add RGE Export to the blender 2.63 Scene panel
#
# tested with the blender 2.63  on 64-bit osx


# ##### BEGIN GPL LICENSE BLOCK #####
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software Foundation,
#  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# ##### END GPL LICENSE BLOCK #####
#
#

bl_info = {
    "name": "RGE Exporter",
    "author": "Yuhei Akamine",
    "version": (2, 7, 2),
    "blender": (2, 7, 2),
    "api": 35622,
    "location": "Scene panel",
    "description": "Export the active scene to RGE scene files",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Import-Export"}

#This script exports active scene to RGE scene files including:
#  mesh
#    -normals, colours, and texture coordinates per face
#  materials 
#    -diffuse, specular, emission 
#    -textures(diffuse only) 
#  lamps
#    -spot, point
#  cameras
#  objects
#    -position, rotation, size
#  F-curves
#    -position, rotation, size
#  pyhisical properties
#    -rigid body, mass
#    -collision bounds type
#       -trimesh, convex hull, cylinder, sphere, box
#       
#       
# !!Important!!:
#    PLEASE DO NOT USE SLASHES(/) and COMMAS(,) in the name of bones.


import bpy
import math
import os
from math import degrees



from bpy.props import *

import mathutils
from mathutils import Matrix

import re

class SCENE_PT_ExportRGE(bpy.types.Operator):
    '''Export a scene objects to a RGE scene file with normals, colors, texture uv, object's transformation, lamps, cameras and phyisical properties.'''
    bl_idname = "export.rge"
    bl_label = "RGE scene files (.rge)"
    bl_register = True

    path = StringProperty(name="RGE Path", description="Filepath used for exporting the RGE file", maxlen=1024, default="test", subtype="FILE_PATH")
#    check_existing = BoolProperty(name="Check Existing", description="Check and warn on overwriting existing files", default=True, options={'HIDDEN'})
    abs_tex_path = BoolProperty(name = "Absolute texture path")

    curve_name = {}
    curve_name["rotation_euler"] = "ROT"
    curve_name["location"]       = "LOC"
    curve_name["scale"]          = "SCA"
    curve_name["rotation_quaternion"] = "ROTQ"

    
    mu = mathutils
    bpy_path = bpy.path
    bpy_data = bpy.data

    os_path = os.path
    
    reg = re
    
    
    def writeRowTransform(self, indent, pose, start, end):
        f = self.file
        
        f.write(indent + "RawTrans: \n")
        f.write(indent + "  Range: [ %d, %d ]\n" % (start, end))

        bone_names = []
        for b in pose.bones:
            bone_names.append(b.name)
        
        f.write(indent + "  Bones: [ %s ]\n" % (", ".join(bone_names)))
        
        
        f.write(indent + "  Trans: ")
                    
        frame_current = self.scene.frame_current
        
        for i in range(start, end):
            self.scene.frame_set(i)
        
            for b in pose.bones:    
                mat = b.matrix
                mat = mat.decompose()
                f.write("  %0.3f %0.3f %0.3f" % tuple(mat[0]))
                f.write(" %0.3f %0.3f %0.3f %0.3f" % tuple(mat[1]))
    
        f.write("\n")
            
        self.scene.frame_current = frame_current
        
        return
    
    def writePoseBone(self, b, idt):
        f = self.file
        
        f.write(idt+" - Name: %s\n" % b.name)
        f.write(idt+"   Length: %f\n" % b.length)

#        head = b.head
        
        pbm = self.mu.Matrix()
        
        if b.parent:
#            head = self.mu.Vector([0, 0, b.parent.length]) * b.parent.bone.matrix
            pbm = b.parent.bone.matrix_local.copy()
        
    #    f.write("%f %f %f" % tuple(head))
  #      bm =  self.mu.Matrix.Translation(head) * b.bone.matrix.to_4x4() 
        bm = pbm.inverted() * b.bone.matrix_local
        
        f.write(idt+"   Matrix:\n")
        for v in bm:
            f.write(idt+"    - [%f, %f, %f, %f]\n" % tuple(v) )   
        
        if b.rotation_mode == "QUATERNION":
            f.write(idt+"   Quat: [%f, %f, %f, %f]\n" % tuple(b.rotation_quaternion))
             
        f.write(idt+"   Bones:\n")
        for b in b.children:            
            self.writePoseBone(b, idt+"  ")
    
    def writePose(self, idt, pose):
        f = self.file
        
        f.write(idt+"Pose:\n")
        
        f.write(idt+"  Bones:\n")    
        
        for b in pose.bones:
            if not b.parent:
                self.writePoseBone(b, idt+" ")
        
        return
    
    def writeIpoCurve(self, indent, name, icu, scale = 1):
        if icu == None:
            return
        
        file = self.file
        
        file.write(indent+"  "+name+":\n")
        for bp in icu.bezierPoints:
            file.write(indent+"    - [ [%f, %f], [%f, %f], [%f, %f] ]\n" % \
            (bp.vec[0][0], bp.vec[0][1]*scale, bp.vec[1][0], bp.vec[1][1]*scale, bp.vec[2][0], bp.vec[2][1]*scale) )
    
    
    def writeFCurve(self, indent, curve, method):
        file = self.file
            
        name = ""
        object = "object"
        if curve.data_path in self.curve_name:
            name = self.curve_name[curve.data_path]
        else:
            path = curve.data_path

            if path.find("pose.bones[") != 0:
                return
                        
            #print(path)
            
            p = self.reg.findall(r'".*"', path)
            
            if len(p) == 0:
                return
            
            bone_name = p[0].replace('"','')            
            #print(bone_name)
            
            p = self.reg.sub(r'".*"', "", path).split(".")
            if len(p) != 3:
                return
            
            object = p[0] + "/" + bone_name
            name = p[2]

            if name in self.curve_name:
                name = self.curve_name[name]
            
        if len(name) == 0:
            return
    
        file.write(indent+"  - "+object+" "+name+" %d" % curve.array_index)
       
        r =curve.range()
        
        file.write(" %d %d" % (r[0], r[1]))

        if method == "SAMPLED_VALUE":
            for i in range(int(r[0]), int(r[1])):
                file.write(" %0.3f" % curve.evaluate(i) )
        else:
            for kp in curve.keyframe_points:
                file.write(" %f %f %f %f %f %f" % \
                (kp.handle_left[0], kp.handle_left[1], kp.co[0], kp.co[1], kp.handle_right[0], kp.handle_right[1]))
        
        file.write("\n")

          
                
    def writeAction(self, action):
        file = self.file
        
        
        fcurves = action.fcurves
    
        file.write("- Name: %s\n" % action.name)
        file.write("  Method: "+action.rge_export_animation_method+"\n")
            
        range = action.frame_range
        file.write("  Range: [ %d, %d ]\n" % (int(range[0]), int(range[1])))
        
    #    print(obj.rge_export_animation_method)
    
        
        file.write("  Curves:\n");
        for curve in fcurves:
            self.writeFCurve(" ", curve, action.rge_export_animation_method )        
    
    
    def writeMesh(self, mesh):
        file = self.file
        
        if(mesh.users == 0):
            return
        
        file.write("  - Name: " + mesh.name + "\n")
            
        vertexGroup = False
        
        for v in mesh.vertices:
            if len(v.groups) > 0:
                vertexGroup = True                
    
        faceUV = (len(mesh.uv_textures) > 0)
        #vertexUV = (len(mesh.sticky) > 0)
        vertexColors = len(mesh.vertex_colors) > 0
    
        if faceUV:
            active_uv = None
            
            for uv_tex in mesh.tessface_uv_textures:
                if uv_tex.active:
                    active_uv  = uv_tex.data
                    break
                
            if active_uv is None:
                faceUV = False
    
        if vertexColors:
            active_col = mesh.vertex_colors[0].data
            if active_col is None:
                vertexColors = False
    
    #    hasCol = len(mesh.vertex_colors) is not 0
    #    hasUV  = mesh.faceUV
    
        
        if not vertexGroup:
            file.write("    Positions: ")
            for v in mesh.vertices:
                file.write("%f %f %f " % tuple(v.co) )

        else:
            file.write("    GroupedPositions: ")        
            for v in mesh.vertices:
                file.write("%d " % len(v.groups))
                for g in v.groups:
                    file.write("%d %f " % (g.group, g.weight))
                file.write("%f %f %f  " % tuple(v.co) )
                  
        file.write("\n")
    
    #   elems = ["v"]   
    #   if hasCol: elems.append("col")
    #   if hasUV : elems.append("uv")       
    #   file.write("    Format: [", ", ".join(elems), "]"   

          
        file.write("    Faces: ")

        data_types = "vn"            

        if vertexColors:
            data_types += "c"
            
        if faceUV :
            data_types += "u"
            
        file.write(data_types + " ")
        
        mesh.calc_tessface()
        for i, face in enumerate(mesh.tessfaces) :
            vs = face.vertices
            #print dir(face)
            
            if len(mesh.materials) > 0 and mesh.materials[face.material_index]:
                file.write("%s " % mesh.materials[face.material_index].name)
            else: 
                file.write("__nomat ")
            
            if not face.use_smooth:
                n = tuple(face.normal)
   
            if faceUV:
                uv  = active_uv[i]
                uv  = uv.uv1, uv.uv2, uv.uv3, uv.uv4 
    
            if vertexColors:
                col = active_col[i]
                col = col.color1, col.color2, col.color3, col.color4
            
            index = 0
            facestr = []
            
            file.write("%d " % len(vs))
#            print(dir(vs))
            
            for j, vidx in enumerate(vs):
                elems = ["%d " % vidx]
    
                if face.use_smooth:
                    v = mesh.vertices[vidx]
                    n = tuple(v.normal)
                elems.append("%f %f %f " % n)
    
                if vertexColors:
                    c = col[j]
                    elems.append("%f %f %f " % tuple(c))
                    
                if faceUV:
                    elems.append("%f %f " % tuple(uv[j]))
                
                
                    
                facestr.append("".join(elems));
                index += 1
            file.write("%s " % "".join(facestr) )
        file.write("\n")
    
    def writeBone(self, bone, indent):
        f = self.file
        
        f.write(indent+"- Name: %s\n" % bone.name)
           
    
    
    obj_type_conv = { "MESH":"Mesh", "LAMP":"Lamp", "CAMERA":"Camera", "ARMATURE":"Armature", "EMPTY":"Empty" }
    
    def writeObject(self, obj, depth=0):
        file = self.file
        
        if not obj.type in self.obj_type_conv:
            return
        
        indent = "";
        for i in range(depth):
            indent += "    "
        
        file.write(indent+"  - Name: %s\n" % obj.name)
            
     #   mat = obj.matrix.copy()
        loc = tuple(obj.location)
        rot = tuple(obj.rotation_euler)
        sca = tuple(obj.scale)
        
    #    obj.location = (0,0,0)
    #     obj.rotation_euler = (0,0,0)
     #   obj.scale = (1,1,1)
    
    #    obj.update()
    
    #   for row in obj.matrixLocal:         
    #       file.write("%s %f %f %f %f" % (indent, row.x, row.y, row.z, row.w)  
    # matrixLocal does'nt work correctly. the following codes get local matrix   
    # fixed at 2.5.5
    
        file.write(indent+"    Matrix:\n")
    
        mat = mathutils.Matrix()
    #    matp = mathutils.Matrix()
    
        mat = mat * mathutils.Matrix.Translation(obj.location)
        mat = mat * obj.rotation_euler.to_matrix().to_4x4()
        mat = mat * mathutils.Matrix.Scale(obj.scale[0], 4, self.mu.Vector((1,0,0)) )
        mat = mat * mathutils.Matrix.Scale(obj.scale[1], 4, self.mu.Vector((0,1,0)) )
        mat = mat * mathutils.Matrix.Scale(obj.scale[2], 4, self.mu.Vector((0,0,1)) )
    
    #        matp = obj.parent.matrix.copy()
    #        matp.invert()
#        print(obj.matrix_local)

        mat = obj.matrix_local.copy() * mat.inverted()

#        if obj.parent != None:
#            mat = obj.parent.matrix_world.copy().invert() * mat
			                     
        if obj.parent_type == "BONE" and obj.parent and obj.parent_bone != "":
            mat = obj.parent.pose.bones[obj.parent_bone].matrix.copy().inverted() * mat


        m =  mat
        file.write(indent+"      - [ %f, %f, %f, %f ]\n" % (tuple(m[0])))
        file.write(indent+"      - [ %f, %f, %f, %f ]\n" % (tuple(m[1])))
        file.write(indent+"      - [ %f, %f, %f, %f ]\n" % (tuple(m[2])))
        file.write(indent+"      - [ %f, %f, %f, %f ]\n" % (tuple(m[3])))
    
        
        file.write(indent + "    Pos: [ %f, %f, %f ]\n" % (loc) )
        file.write(indent + "    Rot: [ %f, %f, %f ]\n" % (rot) )
        file.write(indent + "    Sca: [ %f, %f, %f ]\n" % (sca) )
    
        file.write(indent+"    Type: %s\n" % self.obj_type_conv[obj.type] )

        mesh = obj.data
        if mesh is not None:
            file.write(indent+"    DataName: %s\n" % mesh.name )
    #   else:
    #       file.write(indent+"    none none"
        
        props = []
    
        ph = obj.game
    
        if ph.physics_type is not 'NO_COLLISION':
            col_bounds = ph.collision_bounds_type.capitalize()
            mass = ph.mass
    
            dynamic = int(ph.physics_type is 'RIGID_BODY')
    
            props.append("Mass: %f" % mass);
            props.append("Bounds: %s" % col_bounds);
            props.append("Dynamic: %d" % dynamic);
            
        file.write(indent+"    Props: { %s }\n" % ", ".join(props) )
        
        layer_mask = 0
    
        for i, l in enumerate(obj.layers):
            layer_mask |= int(l) << i
    
        file.write(indent+"    Layers: %d\n" % layer_mask )
        
        if obj.animation_data and obj.animation_data.action:
            if obj.rotation_mode != "XYZ":
                print("RGE Exporter: object's rotation_mode must be XYZ Euler(%s)\n" % obj.name)
            else:
                file.write(indent+"    Action: %s\n" % obj.animation_data.action.name)
        
        if obj.rge_export_bone_anim_method == "ROW_TRANSFORM":
            self.writeRowTransform(indent = indent+"    ", pose = obj.pose, start = obj.rge_export_bone_anim_start, end =  obj.rge_export_bone_anim_end)
        
        if obj.pose:
            self.writePose(indent+"    ", obj.pose)
        
        file.write(indent+"    ParentType: %s\n" % obj.parent_type)
        if obj.parent_bone:
            file.write(indent+"    ParentBone: %s\n" % obj.parent_bone)
        
        children = obj.children
        
        if len(children) is not 0:          
            file.write(indent+"    Children:\n" )
        for child in children:
            self.writeObject(child, depth + 1)
            
    def getRelPath(self, path, base):
        (org_path, fname) = os.path.split(path)
        (base_path, bname)= os.path.split(base)
        
        if org_path == base_path:
            return fname
        else:
            return path
                
    def writeTexture(self, tex):
        file = self.file
        
        if tex is None:
            return
		
       # print (tex.name)
        file.write("  - Name: %s\n" % tex.name )
        file.write("    Type: %s\n" % tex.type )
        if tex.type == "IMAGE":
            dir = self.os_path.dirname(self.path)
            fpath = tex.image.filepath
            
            if not self.abs_tex_path:
                fpath = self.bpy_path.relpath(fpath, dir).replace("//","")
            else:
                fpath = self.bpy_path.abspath(fpath)
            
            file.write("    File: %s\n" % fpath )
    
    def writeMaterial(self, mat):
        file = self.file

        dcol = mat.diffuse_color
        scol = mat.specular_color
        si   = mat.specular_intensity
        file.write("  - Name    : %s\n" % mat.name )
        file.write("    Diffuse : [%f, %f, %f]\n" % (dcol[0], dcol[1], dcol[2]) )
        file.write("    Specular: [%f, %f, %f]\n" % (scol[0]*si, scol[1]*si, scol[2]*si) )
        file.write("    SpecDeg : %f\n" % (mat.specular_hardness) )
        file.write("    Alpha   : %f\n" % (mat.alpha) )
        file.write("    Emit    : %f\n" % (mat.emit) )
            
        useTexture = False;
        for tex_slot in mat.texture_slots:
            if tex_slot is not None and tex_slot.use and tex_slot.use_map_color_diffuse:
                tex = tex_slot.texture
#                print(tex.type)
                if tex is not None and tex.type == "IMAGE":
                    file.write("    Texture : %s\n" % tex.name )
    
            
    def writeCamera(self, cam):
        file = self.file
        
        file.write("  - Name: %s\n" % cam.name )
        file.write("    Clip: [ %f, %f ]\n" % (cam.clip_start, cam.clip_end) )
        file.write("    Lens: %f\n" % cam.lens )
    
    
    lamp_type_conv = { "SPOT":"Spot", "POINT":"Point", "SUN":"Directional", "HEMI":"Hemi", "AREA":"Area", "PHOTON":"Photon"}
    
    
    
    def writeLight(self, light):
        file = self.file
    #    type_names = ("Point", "Directional", "Spot", "Hemi", "Area", "Photon")
    
        file.write("  - Name    : %s\n" % light.name )
        file.write("    Type    : %s\n" % self.lamp_type_conv[light.type] )
        file.write("    Color   : [ %f, %f, %f ]\n" % tuple(light.color) )
        file.write("    Energy  : %f\n" % light.energy )
        
        if light.type == "SPOT":
            file.write("    SpotSize: %f\n" % (light.spot_size * 180.0 / 3.14159))
            file.write("    SpotSoft: %f\n" % light.spot_blend)
            file.write("    Distance: %f\n" % light.distance )
        else:
            file.write("    SpotSize: %f\n" % 1 )
            file.write("    SpotSoft: %f\n" % 1 )
            file.write("    Distance: %f\n" % 1 )
			

    meshes = {}   
    materials = {}
    textures = {}
    cameras = {}
    lights = {}
    actions = {}

    type_to_dic = {'LAMP':lights, 'CAMERA':cameras, 'MESH':meshes }
    
    def collectData(self, obj):
        
        obj.name = obj.name.replace(" ", "_")

        if obj.type in self.type_to_dic:
            cont = self.type_to_dic[obj.type]
            cont[obj.name] = obj.data
            obj.data.name = obj.data.name.replace(" ", "_")
                            
        for ms in obj.material_slots:
            if ms is None:
                continue
            mat = ms.material
            
            mat.name = mat.name.replace(" ", "_")
            
            #print(ms)
            self.materials[mat.name] = mat
            
            for tex_slot in mat.texture_slots:
                if tex_slot is not None:
                    tex = tex_slot.texture
                
                    tex.name = tex.name.replace(" ", "_")
                    self.textures[tex.name] = tex
        
        if obj.animation_data and obj.animation_data.action:
            self.actions[obj.animation_data.action.name] = obj.animation_data.action
        
        for child in obj.children:
            self.collectData(child)
    
    def writeScene(self, filename, scene): 
        f = open(filename, 'w')
        if f is None:
            Blender.Draw.PupMenu("Can't open file")
            return

        #scene.update()
        self.scene = scene
        

        for obj in scene.objects:
            if obj.parent is None:    
                self.collectData(obj)
                
        for action in self.bpy_data.actions:
            action.name.replace(" ", "_")
            if action.use_fake_user:
                self.actions[action.name] = action
        
        self.file = f
                
        f.write('%Blender RGE exporter 2.6.1\n')
        
        if len(self.textures.values()) > 0:
            f.write("Textures:\n")
            for tex in self.textures.values():
                self.writeTexture(tex)
            
        if len(self.materials.values()) > 0:
            f.write("Materials:\n")
            for mat in self.materials.values():
                self.writeMaterial(mat)
                
        meshes = self.meshes
        
        if len(meshes.values()) > 0:
            f.write("Meshes:\n")
            for mesh in meshes.values():
                self.writeMesh(mesh)
            
        if len(self.cameras.values()) > 0:
            f.write("Cameras:\n")
            for cam in self.cameras.values():
                self.writeCamera(cam)
        
        if len(self.lights.values()) > 0:
            f.write("Lights:\n")
            for light in self.lights.values():
                self.writeLight(light)
        
        root_obj = []
        for obj in scene.objects:
            if obj.parent is None:
                root_obj.append(obj)
        
        if len(self.actions.values()) > 0:
            f.write("Actions:\n")
            for action in self.actions.values():
                self.writeAction(action)
            
        f.write("Objects:\n")
    #   print >>f ,"Root:", len(root_obj)
        for obj in root_obj:
            self.writeObject(obj)
        
        f.write("Fps: %d\n" % scene.render.fps)
        
        f.write("%end of file\n")
        f.close()



#    def poll(self, context):
#       return context.active_object != None

    def execute(self, context): 
#        print("Selected: " + context.active_object.name)


#        print(self.properties.path)
        if not self.properties.path:
            raise Exception("filename not set")

        self.writeScene(self.properties.path, context.scene)

        return {'FINISHED'}

    def invoke(self, context, event):
#        wm = context.manager
#        wm.filename = "tsets"
#        wm.add_fileselect(self)
#        return {'RUNNING_MODAL'}
#        print("invoke")
        return self.execute(context)

    def draw(self, context):
        layout = self.layout
        props = self.properties
        print("draw")
#        row = layout.row()
#        row.prop(props, "use_modifiers")
#        row.prop(props, "use_normals")
#        row = layout.row()
#        row.prop(props, "use_uvs")
#        row.prop(props, "use_colors")


        
#def register():
#    bpy.types.register(ExportRGE)


#def unregister():
#    bpy.types.unregister(ExportRGE)
    
#if __name__ == "__main__":
#    unregister()
#    register()
 
 

class SCENE_PT_SetExportPath(bpy.types.Operator):
    '''Make rge path automatically'''
    bl_idname = "make_export_path.rge"
    bl_label = "Make file name"

    def invoke(self, context, event):
        print (bpy.data.filepath)
        context.scene.rge_path = bpy.data.filepath.replace(".blend",".rge")
        return {'FINISHED'}
    

        
class SCENE_PT_rge_exporter(bpy.types.Panel):
    bl_label = "Export to RGE"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "scene"

    bpy_path = bpy.path
        
    def draw(self, context):
                  
        layout = self.layout

        scene = context.scene

        row = layout.row()
        row.label(text="Exporting scene is: " + scene.name)
                   
        row = layout.row()
        row.label(text="Blend path is: "+context.blend_data.filepath)
        
        row = layout.row()
       
        rge = row.operator("export.rge", text="Export to rge")
        rge.path = self.bpy_path.abspath(scene.rge_path)
        rge.abs_tex_path = scene.rge_export_abs_tex_path
        
        #print(rge.path)
        
        row = layout.row()  
        row.prop(scene, "rge_path")
        
        row = layout.row()
        row.operator("make_export_path.rge", text="Make file name")

        row = layout.row()
        row.prop(scene, "rge_export_abs_tex_path")

class OBJECT_PT_rge_exporter(bpy.types.Panel):
    bl_label = "RGE Exporter"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    bpy_path = bpy.path
        
    def draw(self, context):
                  
        layout = self.layout

        obj = context.object


        if obj.animation_data:
            row = layout.row()
            row.prop(obj.animation_data, "action")

            if obj.animation_data.action:
                row = layout.row()
                row.label(text = "Method to export animation")
        
                row = layout.row()
                row.prop(obj.animation_data.action, "rge_export_animation_method")
                
                a = obj.animation_data.action
                row = layout.row()
                row.label( text = "Animation range (%d,%d)" %tuple(a.frame_range))
            else:
                row = layout.row()
                row.label(text = "No action")
                
        else:
            row = layout.row()
            row.label(text = "No animation")

        layout.row().separator()


            
        if obj.type == "ARMATURE":
            row = layout.row()
            row.label(text = "Export method for bone's action")
            
            row = layout.row()
            row.prop(obj, "rge_export_bone_anim_method")
            
            if obj.rge_export_bone_anim_method == "ROW_TRANSFORM":
                row = layout.row()
                col = row.column()
                col.prop(obj, "rge_export_bone_anim_start",text = "start")
                col = row.column()
                col.prop(obj, "rge_export_bone_anim_end", text = "end")



def register():
    bpy.utils.register_module(__name__)
    pass


def unregister():
    bpy.utils.unregister_module(__name__)
    pass

if __name__ == "__main__":
    register()
    
bpy.types.Scene.rge_path = StringProperty(name = "rge path", subtype = "FILE_PATH", default = "noname.rge")
bpy.types.Scene.rge_export_abs_tex_path = BoolProperty(name = "absolute texture path", default = True)

e = []
e.append(tuple(["F-Curve export type","F-Curve export type","CONTROL_POINTS"]))
e.append(tuple(["CONTROL_POINTS","Control Points","CONTROL_POINTS"]))
e.append(tuple(["SAMPLED_VALUE","Value at every frame","SAMPLED_VALUE"]))
#e.append(tuple(["SAMPLED_MATRIX", "Local matrix at every frame", "SAMPLED_MATRIX"]))

bpy.types.Action.rge_export_animation_method =  EnumProperty(items = e, description = "fcurve export type", default = "CONTROL_POINTS")

e = []
e.append(tuple(["Export method for bone's action","Export method for bone's action","CONTROL_POINTS"]))
e.append(tuple(["F-CURVES","F-Curves","F-CURVES"]))
e.append(tuple(["ROW_TRANSFORM","Value affected by constraints","ROW_TRANSFORM"]))

bpy.types.Object.rge_export_bone_anim_method =  EnumProperty(items = e, description = "bone's action export type", default = "F-CURVES")
bpy.types.Object.rge_export_bone_anim_start = IntProperty(description = "start frame of exporting animation", default = 1)
bpy.types.Object.rge_export_bone_anim_end = IntProperty(description = "end frame of exporting animation", default = 250)
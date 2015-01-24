/*
 *  MeshObj.cpp
 *  collada_test
 *
 *  Created by Yuhei Akamine on 07/06/03.
 *  Copyright 2007 Y. Akamine. All rights reserved.
 *
 */
#include <iostream>
#include <limits>
using namespace std;

#include "RGE.h"
#include "MeshObj.h"

using namespace rge;

void CallListMesh::beginRecord()
{
    mCallList = glGenLists(1);
    
    glNewList(mCallList, GL_COMPILE);
}

void CallListMesh::endRecord()
{
    glEndList();
}

void CallListMesh::render()
{
    if(mCallList > 0)
        glCallList(mCallList);
    
    glDisable(GL_COLOR_MATERIAL);
}

void TriMeshObject::readVertexFromRgm(std::istream& is, bool bflat,
        int& pi, rgeVector3& normal, color4& color, rgeVector2& uv) {
    string item;

    if (bflat)
        is >> item >> pi;
    else
        is >> item >> pi >> item >> normal;

    if (mSource & TYPE_COLOR) {
        color3 col;
        is >> item >> col;
        if (item != "c")
            throw "Error at color in Faces";

        col = col / 256.0;
        color = col;
    }
    if (mSource & TYPE_TEXCOORD) {
        rgeVector2 uv;
        is >> item >> uv;
        if (item != "uv")
            throw "Error at uv in Faces";
    }
}

void TriMeshObject::readVerticesFromRgm(std::istream& is) {
    typedef map<string, int> str2int;
    str2int mat2imesh;

    char buf[1024];
    string item, arg1;
    int i, n, count;
    //	bool hasUV = false, hasColor = false;

    is >> item;
    is.getline(buf, sizeof (buf));
    stringstream line(buf);
    line >> arg1;
    setId(arg1);

    //	mHasUV = mHasColors = false;
    mSource = TYPE_POSITION | TYPE_NORMAL;

    while (!line.eof()) {
        line >> item;
        if (item == "u")
            mSource |= TYPE_TEXCOORD;
            //			mHasUV =  true;
        else if (item == "c")
            mSource |= TYPE_COLOR;
        //			mHasColors =  true;
    }

    is >> item >> count;
    if (item != "Position")
        throw "Error at Position";
    for (i = 0; i < count; ++i) {
        rgeVector3 pos;
        is >> pos;
        mPositions.push_back(pos);
    }

    //	mesh_map tri_mesh_list, quad_mesh_list;

    is >> item >> count;
    if (item != "Faces")
        throw "Error at Faces";
    for (i = 0; i < count; ++i) {
        int imesh;
        string mat, smooth;
        rgeVector3 normal;
        rgeVector2 uv;
        color3 color;

        bool bflat = false;
        is >> mat >> smooth;

        int pi1, pi2, pi3, pi4;
        rgeVector3 no1, no2, no3, no4;
        rgeVector2 uv1, uv2, uv3, uv4;
        color4 co1, co2, co3, co4;

        if (smooth == "flat") {
            is >> item >> normal;
            bflat = true;

            no1 = no2 = no3 = no4 = normal;
        }

        is >> n;

        if (mat2imesh.find(mat) == mat2imesh.end()) {
            imesh = makeTriangleMesh(mat);
            mat2imesh.insert(str2int::value_type(mat, imesh));
        } else {
            imesh = mat2imesh[mat];
        }


        if (n == 3) {
            readVertexFromRgm(is, bflat, pi1, no1, co1, uv1);
            readVertexFromRgm(is, bflat, pi2, no2, co2, uv2);
            readVertexFromRgm(is, bflat, pi3, no3, co3, uv3);

            addTriangle(imesh,
                    pi1, no1, co1, uv1,
                    pi2, no2, co2, uv2,
                    pi3, no3, co3, uv3);
        } else {
            readVertexFromRgm(is, bflat, pi1, no1, co1, uv1);
            readVertexFromRgm(is, bflat, pi2, no2, co2, uv2);
            readVertexFromRgm(is, bflat, pi3, no3, co3, uv3);
            readVertexFromRgm(is, bflat, pi4, no4, co4, uv4);

            addQuad(imesh,
                    pi1, no1, co1, uv1,
                    pi2, no2, co2, uv2,
                    pi3, no3, co3, uv3,
                    pi4, no4, co4, uv4);
        }
    }
}

void TriMeshObject::createFromRgm(std::istream& is) {
    mesh_map tri_mesh_list;

    readVerticesFromRgm(is);

    /*
    for(mesh_map::iterator i=tri_mesh_list.begin();
            i!=tri_mesh_list.end();
            ++i) {
            mesh& m = i->second; 
		
            getRGE()->logger() << "\ttri mesh: " << m.indices.size() << " triangles of "<< m.material_name << "\n";
            m.material = getRGE()->findMaterial(m.material_name);
            if(m.material.get()==0) 
                    getRGE()->logger() << "Mesh: can't resolve material " << m.material_name << "\n";
		
            //join quads having same material as tri_mesh i
            mesh_map::iterator q = quad_mesh_list.find(i->first);
            if(q != quad_mesh_list.end()) {
                    mesh& qm = q->second;
    //			getRGE()->logger() << "\tquad mesh converting tri: " << qm.indices.size() << " quads of "<< m.material_name << "\n";
			
                    for(indexSets::iterator qi = qm.indices.begin(); qi != qm.indices.end(); ) {
                            const indexSet& i1 = *(qi++);
                            const indexSet& i2 = *(qi++);
                            const indexSet& i3 = *(qi++);
                            const indexSet& i4 = *(qi++);
				
                            m.indices.push_back(i1);
                            m.indices.push_back(i2);
                            m.indices.push_back(i3);

                            m.indices.push_back(i1);
                            m.indices.push_back(i3);
                            m.indices.push_back(i4);
                    }
                    qm.indices.clear();
            }

            mTriMeshes.push_back(m);	
    }
    // read quads as triangles
    for(mesh_map::iterator i=quad_mesh_list.begin();
            i!=quad_mesh_list.end();
            ++i) {
            mesh& m = i->second; 
            getRGE()->logger() << "\tquad mesh converting tri: " << m.indices.size() << " quads of "<< m.material_name << "\n";
		
            m.material = getRGE()->findMaterial(m.material_name);
            if(m.material.get()==0) 
                    getRGE()->logger() << "Mesh: can't resolve material " << m.material_name << "\n";
		
            indexSets indices;
            {
                    getRGE()->logger() << "\tquad mesh converting tri: " << m.indices.size() << " quads of "<< m.material_name << "\n";
			
                    for(indexSets::iterator qi = m.indices.begin(); qi != m.indices.end(); ) {
                            const indexSet& i1 = *(qi++);
                            const indexSet& i2 = *(qi++);
                            const indexSet& i3 = *(qi++);
                            const indexSet& i4 = *(qi++);
				
                            indices.push_back(i1);
                            indices.push_back(i2);
                            indices.push_back(i3);
				
                            indices.push_back(i1);
                            indices.push_back(i3);
                            indices.push_back(i4);
                    }
            }		
            m.indices = indices;
		
            mTriMeshes.push_back(m);
    }
     */
}

void TriMeshObject::resolveMaterials() {
    for (vector<mesh>::iterator tri = mTriMeshes.begin(); tri != mTriMeshes.end(); ++tri) {
        tri->material = getRGE()->findMaterial(tri->material_name);

        if (tri->material.get())
            getRGE()->logger() << "resolve material name:" << tri->material_name << "\n";
        else
            getRGE()->logger() << "unresolvalbe material name:" << tri->material_name << "\n";
    }
}

int TriMeshObject::makeTriangleMesh(MaterialRef material) {
    mesh m;
    m.material = material;

    mTriMeshes.push_back(m);

    return mTriMeshes.size() - 1;
}

int TriMeshObject::makeTriangleMesh(const string& mat_name) {
    mesh m;
    m.material_name = mat_name;

    mTriMeshes.push_back(m);

    return mTriMeshes.size() - 1;
}

void TriMeshObject::setMeshMaterial(MaterialRef mat) {
    int imesh=-1;
    for (vector<mesh>::iterator tri = mTriMeshes.begin(); tri != mTriMeshes.end(); ++tri) {
        if(tri->material == mat)
            imesh = distance(mTriMeshes.begin(), tri);
    }
    if(imesh<0)
        imesh = makeTriangleMesh(mat);
    
    mCurrentMesh = imesh;
}

void TriMeshObject::setMeshMaterial(int imat) {
    mCurrentMesh = imat;
}
        

void TriMeshObject::addVertex(int iMesh, int iVertex1, const rgeVector3& normal1, const color4& col1, const rgeVector2& uv1) {
    mTriMeshes[iMesh].positions.push_back(iVertex1);
    mTriMeshes[iMesh].normals.push_back(normal1);
    mTriMeshes[iMesh].colors.push_back(col1);
    mTriMeshes[iMesh].uvs.push_back(uv1);
}

void TriMeshObject::addTriangle(int iMesh,
        int iVertex1, const rgeVector3& normal1, const color4& col1, const rgeVector2& uv1,
        int iVertex2, const rgeVector3& normal2, const color4& col2, const rgeVector2& uv2,
        int iVertex3, const rgeVector3& normal3, const color4& col3, const rgeVector2& uv3
        ) {
    addVertex(iMesh, iVertex1, normal1, col1, uv1);
    addVertex(iMesh, iVertex2, normal2, col2, uv2);
    addVertex(iMesh, iVertex3, normal3, col3, uv3);
}

void TriMeshObject::addTriangle(int iMesh, int i1, int i2, int i3)
{
    rgeVector3 normal = calcNormal(i1, i2, i3);
    color3 c(1,1,1);
    rgeVector2 uv(0,0);
    
    addVertex(iMesh, i1, normal, c, uv);
    addVertex(iMesh, i2, normal, c, uv);
    addVertex(iMesh, i3, normal, c, uv);
}

void TriMeshObject::getBoundingBox(rgeVector3& _max, rgeVector3& _min) {
    double lowest = numeric_limits<double>::min();
    double highest = numeric_limits<double>::max();

    _max = rgeVector3(lowest, lowest, lowest);
    _min = rgeVector3(highest, highest, highest);

    for (vector<mesh>::iterator tri = mTriMeshes.begin(); tri != mTriMeshes.end(); ++tri) {
        for (ints::iterator i = tri->positions.begin(); i != tri->positions.end(); ++i) {
            if (*i >= 0) {
                rgeVector3& v = mPositions[*i];

                _max.x = std::max(_max.x, v.x);
                _max.y = std::max(_max.y, v.y);
                _max.z = std::max(_max.z, v.z);

                _min.x = std::min(_min.x, v.x);
                _min.y = std::min(_min.y, v.y);
                _min.z = std::min(_min.z, v.z);
            }
        }
    }
}

void TriMeshObject::getTriangles(rgeVector3s& v) {
    for (vector<mesh>::iterator tri = mTriMeshes.begin(); tri != mTriMeshes.end(); ++tri) {
        for (ints::iterator i = tri->positions.begin(); i != tri->positions.end();) {
            if (*i >= 0) {
                v.push_back(mPositions[*i++]);
                v.push_back(mPositions[*i++]);
                v.push_back(mPositions[*i++]);
            }
        }
    }
}

void TriMeshObject::getTriangleIndexVertices(rgeVector3s& v, ints& index) {
    v = mPositions;

    for (vector<mesh>::iterator tri = mTriMeshes.begin(); tri != mTriMeshes.end(); ++tri) {
        for (ints::iterator i = tri->positions.begin(); i != tri->positions.end();) {
            if (*i >= 0) {
                index.push_back(*i++);
                index.push_back(*i++);
                index.push_back(*i++);
            }
        }
    }
}

void TriMeshObject::renderTriangles() {
    if (mSource & TYPE_COLOR) {
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT, GL_DIFFUSE);
    } else {
        glDisable(GL_COLOR_MATERIAL);
    }
    for (vector<mesh>::iterator tri = mTriMeshes.begin(); tri != mTriMeshes.end(); ++tri) {

        if (tri->material.get()) {
            if (tri->material->useBlending() != getRGE()->renderBlend())
                continue;
            tri->material->issue();
        }

        glBegin(GL_TRIANGLES);

        for (size_t i = 0; i < tri->positions.size(); i++) {
            if (mSource & TYPE_TEXCOORD) {
                rgeVector2& v = tri->uvs[i];
                glTexCoord2d(v.x, v.y);
            }
            if (mSource & TYPE_COLOR) {
                color4& c = tri->colors[i];
                glColor4f(c.r, c.g, c.b, c.a);
            }
            if (mSource & TYPE_NORMAL) {
                rgeVector3& n = tri->normals[i];
                glNormal3f(n.x, n.y, n.z);
            }

            rgeVector3& v = mPositions[tri->positions[i]];
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
    }
    /*	for(vector<mesh>::iterator tri=mQuadMeshes.begin();tri != mQuadMeshes.end(); ++tri) {
	 
     if(tri->material.get()) {
     if( tri->material->useBlending() != getRGE()->renderBlend() )
     continue;
     tri->material->issue();
     }		
     glBegin(GL_QUADS);
	 
     for(indexSets::iterator i=tri->indices.begin(); i!=tri->indices.end(); ++i) {
     if(i->iTexcoord >= 0) {
     rgeVector2& v = mTexcoords[i->iTexcoord];
     glTexCoord2d(v.x, v.y);
     }
     if(i->iNormal >= 0) {
     rgeVector3& n = mNormals[i->iNormal];
     glNormal3f(n.x, n.y, n.z);
     }
     if(i->iColor >= 0) {
     color4& c = mColors[i->iColor];
     glColor4f(c.r, c.g, c.b, 1);
     //				GLfloat d[] = {c.x,c.y,c.z,1.0};
     //				glMaterialfv(GL_FRONT, GL_DIFFUSE, d);
     }
	 
     if(i->iVertex >= 0) {
     rgeVector3& v = mVertices[i->iVertex];
     glVertex3f(v.x, v.y, v.z);
     }
     }
     glEnd();
     }*/

}

void TriMeshObject::render() {
    bool bMakingList = false;

    GLuint *list = getRGE()->isMaterialEnable() ? &mDisplayList : &mDisplayListNoMaterial;

    if (*list == 0) {
        *list = glGenLists(1);

        if (*list != 0) {
            glNewList(*list, GL_COMPILE_AND_EXECUTE);
            bMakingList = true;
        }
    } else {
        glCallList(*list);
        return;
    }

    renderTriangles();

    glDisable(GL_COLOR_MATERIAL);

    if (bMakingList)
        glEndList();


}

rgeVector3 TriMeshObject::calcNormal(int i1, int i2, int i3) {
    rgeVector3 v1 = mPositions[i1];
    rgeVector3 v2 = mPositions[i2];
    rgeVector3 v3 = mPositions[i3];
    return (v3 - v1).cross(v2 - v1).normalized();
}

void TriMeshObject::translate(const rgeVector3& t)
{
    for(rgeVector3s::iterator i=mPositions.begin(); i!=mPositions.end(); ++i)
    {
        *i += t;
    }
}

#include <stack>
//static float frand(float f=1) { return (float)rand() / RAND_MAX * f; }
/*
void TriMeshObject::decomposeByPlane()
{
        typedef std::stack<Face*> face_stack;
	
        meshes& org_meshes = mTriMeshes;
        meshes decomposed;
	
        typedef std::vector<Face> Faces;
//	typedef std::vector<Faces> FacesAr;
	
        typedef vector<ints> intss;
	
        //頂点が属するFaceを記録する
        intss vertToFace(mVertices.size());	
        for(meshes::iterator m=org_meshes.begin(); m!=org_meshes.end(); ++m) {
                for(int i=0; i!=m->indices.size(); i+=3) {
                        vertToFace[m->indices[i  ].iVertex].push_back(i);
                        vertToFace[m->indices[i+1].iVertex].push_back(i);
                        vertToFace[m->indices[i+2].iVertex].push_back(i);
                }
        }

        //Faceリストを作る
        Faces faces;
        for(meshes::iterator m=org_meshes.begin(); m!=org_meshes.end(); ++m) {
                for(int i=0; i!=m->indices.size(); i+=3) {
                        Face f(m->indices[i], m->indices[i+1], m->indices[i+2],
                                   calcNormal(m->indices[i].iVertex,m->indices[i+1].iVertex,m->indices[i+2].iVertex)
                                  );
			
                        for(int j=0; j<3; ++j) {
                                ints& vtf = vertToFace[m->indices[i+j].iVertex];
                                for(ints::iterator k = vtf.begin(); k!=vtf.end(); ++k) {
                                        if(find(f.adjacents.begin(), f.adjacents.end(), *k/3) 
                                           == f.adjacents.end()) {
                                                f.adjacents.push_back((*k)/3);
                                        }
                                }
                        }
                        faces.push_back(f);
                }
        }
	
	
        //TODO:隣接faceをたどっていき同じ法線を持つfaceのグループを作る．
        //	0. 任意の非選択face F を選ぶ．
        //	1. Fに選択済みマークをつける
        //  2. Fが持つ頂点を含む隣接faceのうち法線が同じface Fnearをスタックに入れる
        //  3. スタックから頂点を取り出し，その頂点を含むfaceのうちマークされていないものをFとする
        //  4. 1へ. スタックが空になったら終了
	
        Face* F = 0;
        while(true) {
                //0.任意の非選択Faceを選ぶ
                int i;
                for(i=0; i!=faces.size(); ++i) {
                        if(!faces[i].marked) 
                                break;
                }
                //すべて選択済みであれば終了
                if(i == faces.size())
                        break;
		
                F = &(faces[i]); 
		
                face_stack same_normal;
		
                mesh m;
                m.material = getRGE()->makeMaterial("temp");
                m.material->create(color4(frand(), frand(), frand(), 1));
		
                while(true) {
                        //1
                        F->marked = true;
			
                        m.indices.push_back(*(F->pi1));
                        m.indices.push_back(*(F->pi2));
                        m.indices.push_back(*(F->pi3));
			
                        //2
                        rgeVector3 Nf = F->normal;
                        for(ints::iterator iFa = F->adjacents.begin(); iFa !=F->adjacents.end(); ++iFa) {
                                Face& Fa = faces[*iFa];
                                if(!Fa.marked &&
                                   (Nf - Fa.normal).length() < 0.01 ) {
                                        same_normal.push(&Fa);
                                }
                        }
			
                        //4
                        if(same_normal.empty())
                                break;
			
                        //3
                        F = same_normal.top();
                        same_normal.pop();
                }
                decomposed.push_back(m);
        }
	
        mTriMeshes = decomposed;
}
 */


/////////
#if 0

void QuadMeshObject::createFromRgm(std::istream& is) {
    mesh_map tri_mesh_list, quad_mesh_list;

    readVerticesFromRgm(is, tri_mesh_list, quad_mesh_list);

    for (mesh_map::iterator i = tri_mesh_list.begin();
            i != tri_mesh_list.end();
            ++i) {
        mesh& m = i->second;
        getRGE()->logger() << "\ttri mesh: " << m.indices.size() << " triangles of " << m.material_name << "\n";
        m.material = getRGE()->findMaterial(m.material_name);
        if (m.material.get() == 0)
            getRGE()->logger() << "Mesh: can't resolve material " << m.material_name << "\n";
        mTriMeshes.push_back(m);
    }
    for (mesh_map::iterator i = quad_mesh_list.begin();
            i != quad_mesh_list.end();
            ++i) {
        mesh& m = i->second;
        getRGE()->logger() << "\tquad mesh: " << m.indices.size() << " quadrangles of " << m.material_name << "\n";
        m.material = getRGE()->findMaterial(m.material_name);
        if (m.material.get() == 0)
            getRGE()->logger() << "Mesh: can't resolve material " << m.material_name << "\n";
        mQuadMeshes.push_back(m);
    }
}

void QuadMeshObject::resolveMaterials() {
    TriMeshObject::resolveMaterials();

    for (vector<mesh>::iterator quad = mQuadMeshes.begin(); quad != mQuadMeshes.end(); ++quad) {
        quad->material = getRGE()->findMaterial(quad->material_name);

        if (quad->material.get())
            getRGE()->logger() << "resolve material name:" << quad->material_name << "\n";
        else
            getRGE()->logger() << "unresolvalbe material name:" << quad->material_name << "\n";
    }
}

void QuadMeshObject::getBoundingBox(rgeVector3& _max, rgeVector3& _min) {
    TriMeshObject::getBoundingBox(_max, _min);

    for (vector<mesh>::iterator tri = mQuadMeshes.begin(); tri != mQuadMeshes.end(); ++tri) {
        for (indexSets::iterator i = tri->indices.begin(); i != tri->indices.end(); ++i) {
            if (i->iVertex >= 0) {
                rgeVector3& v = mVertices[i->iVertex];

                _max.x = max(_max.x, v.x);
                _max.y = max(_max.y, v.y);
                _max.z = max(_max.z, v.z);

                _min.x = min(_min.x, v.x);
                _min.y = min(_min.y, v.y);
                _min.z = min(_min.z, v.z);
            }
        }
    }
}

void QuadMeshObject::getTriangles(rgeVector3s& v) {
    TriMeshObject::getTriangles(v);

    for (vector<mesh>::iterator quad = mQuadMeshes.begin(); quad != mQuadMeshes.end(); ++quad) {
        for (indexSets::iterator i = quad->indices.begin(); i != quad->indices.end();) {
            if (i->iVertex >= 0) {
                rgeVector3& v1 = mVertices[i++->iVertex];
                rgeVector3& v2 = mVertices[i++->iVertex];
                rgeVector3& v3 = mVertices[i++->iVertex];
                rgeVector3& v4 = mVertices[i++->iVertex];
                v.push_back(v1);
                v.push_back(v2);
                v.push_back(v3);

                v.push_back(v1);
                v.push_back(v3);
                v.push_back(v4);
            }
        }
    }
}

void QuadMeshObject::getTriangleIndexVertices(rgeVector3s& v, ints& index) {
    TriMeshObject::getTriangleIndexVertices(v, index);

    for (vector<mesh>::iterator quad = mQuadMeshes.begin(); quad != mQuadMeshes.end(); ++quad) {
        for (indexSets::iterator i = quad->indices.begin(); i != quad->indices.end();) {
            if (i->iVertex >= 0) {
                int i1 = i++->iVertex;
                int i2 = i++->iVertex;
                int i3 = i++->iVertex;
                int i4 = i++->iVertex;

                index.push_back(i1);
                index.push_back(i2);
                index.push_back(i3);

                index.push_back(i1);
                index.push_back(i3);
                index.push_back(i4);
            }
        }
    }
}

void QuadMeshObject::render() {
    bool bMakingList = false;

    GLuint *list = getRGE()->isMaterialEnable() ? &mDisplayList : &mDisplayListNoMaterial;

    if (*list == 0) {
        *list = glGenLists(1);

        if (*list != 0) {
            glNewList(*list, GL_COMPILE_AND_EXECUTE);
            bMakingList = true;
        }
    } else {
        glCallList(*list);
        return;
    }

    renderTriangles();
    for (vector<mesh>::iterator tri = mQuadMeshes.begin(); tri != mQuadMeshes.end(); ++tri) {

        if (tri->material.get()) {
            if (tri->material->useBlending() != getRGE()->renderBlend())
                continue;
            tri->material->issue();
        }
        glBegin(GL_QUADS);

        for (indexSets::iterator i = tri->indices.begin(); i != tri->indices.end(); ++i) {
            if (i->iTexcoord >= 0) {
                rgeVector2& v = mTexcoords[i->iTexcoord];
                glTexCoord2d(v.x, v.y);
            }
            if (i->iNormal >= 0) {
                rgeVector3& n = mNormals[i->iNormal];
                glNormal3f(n.x, n.y, n.z);
            }
            if (i->iColor >= 0) {
                color4& c = mColors[i->iColor];
                glColor4f(c.r, c.g, c.b, 1);
                //				GLfloat d[] = {c.x,c.y,c.z,1.0};
                //				glMaterialfv(GL_FRONT, GL_DIFFUSE, d);
            }

            if (i->iVertex >= 0) {
                rgeVector3& v = mVertices[i->iVertex];
                glVertex3f(v.x, v.y, v.z);
            }
        }
        glEnd();
    }
    glDisable(GL_COLOR_MATERIAL);

    if (bMakingList)
        glEndList();


}

#endif

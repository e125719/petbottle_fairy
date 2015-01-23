/*
 *  MeshObj.h
 *  collada_test
 *
 *  Created by Yuhei Akamine on 07/06/01.
 *  Copyright 2007 Yuhei Akamine. All rights reserved.
 *
 */
#pragma once 
#include <iostream>
#include <memory>

#include <vector>
#include <string>
#include <sstream>
#include <map>
using namespace std;

#include "Types.h"
#include "Renderable.h"

namespace rge {

    const int MAX_SOURCES = 10;

    class Material;
    class RGE;
/*
    class CubeMesh : public Renderable {
    public:

        CubeMesh(GLfloat s) : size(s) {
        }

        virtual void getBoundingBox(rgeVector3& max, rgeVector3& min) {
            max = rgeVector3(size / 2, size / 2, size / 2);
            min = rgeVector3(-size / 2, -size / 2, -size / 2);
        }

        virtual void render() {
            glutSolidCube(size);
        }

        virtual GEOM_TYPE geomType() {
            return PRIMITIVE;
        }

    private:
        GLfloat size;
    };

    class SphereMesh : public Renderable {
    public:

        SphereMesh(GLfloat r) : size(r) {
        }

        virtual void getBoundingBox(rgeVector3& max, rgeVector3& min) {
            max = rgeVector3(size, size, size);
            min = rgeVector3(-size, -size, -size);
        }

        virtual void render() {
            glutSolidSphere(size, 12, 12);
        }

        virtual GEOM_TYPE geomType() {
            return PRIMITIVE;
        }
    private:
        GLfloat size;
    };

    typedef std::shared_ptr<CubeMesh> CubeMeshRef;
*/
    class CallListMesh : public Renderable, public RGEObject
    {
    public:
        CallListMesh( ) : mCallList() {}
        
        void beginRecord();
        void endRecord();
    
        virtual void getBoundingBox(rgeVector3& b , rgeVector3& a ) {}
        virtual void render();
        virtual GEOM_TYPE geomType() {
            return MESH_OBJ;
        }
    private:
        GLuint mCallList;
    };
    
    typedef std::shared_ptr<CallListMesh> CallListMeshRef;
    
    class MeshObject : public Renderable, public RGEObject {
    public:

        MeshObject() : RGEObject() {
        }

        virtual void createFromRgm(std::istream& is) = 0;
        virtual void getTriangles(rgeVector3s& triangles) = 0;

        virtual void translate(const rgeVector3& t) = 0;

        /*	int addNormal(const rgeVector3& n) { mNormals.push_back(n); return mNormals.size()-1; }
                int addColor(const color4& c) { mColors.push_back(c); return mColors.size()-1; }
                int addTexCoord(const rgeVector2& t) { mTexcoords.push_back(t); return mTexcoords.size()-1; }

                int numVertices() { return mVertices.size(); }
                int numNormals() { return mNormals.size(); }
                int numColors() { return mColors.size(); }
                int numTexcoord() { return mTexcoords.size(); }
         */
    protected:

    };
    


    class TriMeshObject : public MeshObject {
    public:
        typedef std::shared_ptr<TriMeshObject> MeshObjectRef;
        typedef std::shared_ptr<Material> MaterialRef;
        typedef int SOURCE_TYPE;

        static const SOURCE_TYPE TYPE_POSITION = 1 << 1;
        static const SOURCE_TYPE TYPE_COLOR = 1 << 2;
        static const SOURCE_TYPE TYPE_TEXCOORD = 1 << 3;
        static const SOURCE_TYPE TYPE_NORMAL = 1 << 4;
        static const SOURCE_TYPE TYPE_ALL = (TYPE_NORMAL << 1) - 1;

        struct mesh {

            mesh() : material() {
            }

            ints positions;
            rgeVector3s normals;
            color4s colors;
            rgeVector2s uvs;

            string material_name;
            MaterialRef material;
        };

        typedef vector<mesh> meshes;
        typedef map<string, mesh> mesh_map;
        /*
        struct Face {
                Face(indexSet& i1, indexSet& i2, indexSet& i3, rgeVector3 n) :
                pi1(&i1), pi2(&i2), pi3(&i3), normal(n), marked(false) {
			
                        //normal = calcNormal(pi1->iVertex, pi2->iVertex, pi3->iVertex);
                }
                indexSet *pi1, *pi2, *pi3;
                rgeVector3 normal;
                bool marked;
		
                ints adjacents;
        };*/

        static const int NO_INDEX = -1;

        TriMeshObject() : MeshObject(),
        mDisplayList(0), mDisplayListNoMaterial(0),
        mSource(TYPE_POSITION) {
        }
        virtual void getBoundingBox(rgeVector3& max, rgeVector3& min);
        virtual void resolveMaterials();
        virtual void render();

        virtual GEOM_TYPE geomType() {
            return TRI_MESH_OBJ;
        }

        virtual void createFromRgm(std::istream& is);
        virtual void getTriangles(rgeVector3s& triangles);

        virtual void getTriangleIndexVertices(rgeVector3s& v, ints& index);

        virtual void clear() {
            mTriMeshes.clear();
            mPositions.clear();
            glDeleteLists(mDisplayList, 1);
            glDeleteLists(mDisplayListNoMaterial, 1);

            mDisplayList = mDisplayListNoMaterial = 0;
        }

        int makeTriangleMesh(MaterialRef material);
        int makeTriangleMesh(const string& mat_name);

        void setMeshMaterial(MaterialRef mat);
        void setMeshMaterial(int imat);
        
        void setSourceType(SOURCE_TYPE s) {
            mSource = s;
        }

        int addPosition(const rgeVector3& v) {
            mPositions.push_back(v);
            return (int)mPositions.size() - 1;
        }

        int numPositions() {
            return (int)mPositions.size();
        }
        void addVertex(int iMesh, int iVertex1, const rgeVector3& normal1, const color4& col1, const rgeVector2& uv1);

        void addTriangle(int iMesh,
                int iVertex1, const rgeVector3& normal1, const color4& col1, const rgeVector2& uv1,
                int iVertex2, const rgeVector3& normal2, const color4& col2, const rgeVector2& uv2,
                int iVertex3, const rgeVector3& normal3, const color4& col3, const rgeVector2& uv3
                );
        void addTriangle(int iMesh, int, int, int);
        void addTriangle(int i1, int i2, int i3)
        {
            addTriangle(mCurrentMesh, i1,i2,i3);
        }
        void addTriangle(const rgeVector3& v1, const rgeVector3& v2, const rgeVector3& v3)
        {
            int i=addPosition(v1);
            addPosition(v2);
            addPosition(v3);
            
            addTriangle(i, i+1, i+2);
        }

        void addQuad(int iMesh,
                int iVertex1, const rgeVector3& normal1, const color4& col1, const rgeVector2& uv1,
                int iVertex2, const rgeVector3& normal2, const color4& col2, const rgeVector2& uv2,
                int iVertex3, const rgeVector3& normal3, const color4& col3, const rgeVector2& uv3,
                int iVertex4, const rgeVector3& normal4, const color4& col4, const rgeVector2& uv4
                ) {
            addTriangle(iMesh,
                    iVertex1, normal1, col1, uv1,
                    iVertex2, normal2, col2, uv2,
                    iVertex3, normal3, col3, uv3);
            addTriangle(iMesh,
                    iVertex1, normal1, col1, uv1,
                    iVertex3, normal3, col3, uv3,
                    iVertex4, normal4, col4, uv4);
        }

        void addQuad(int iMesh, int i1, int i2, int i3, int i4) {
            addTriangle(iMesh, i1, i2, i3);
            addTriangle(iMesh, i1, i3, i4);
        }
        void addQuad(int i1, int i2, int i3, int i4)
        {
            addQuad(mCurrentMesh, i1,i2,i3,i4);
        }

        rgeVector3 calcNormal(int i1, int i2, int i3);
        void decomposeByPlane();
        void translate(const rgeVector3& t);
        
    protected:
        void readVertexFromRgm(std::istream& is, bool bflat,
                int& pi, rgeVector3& normal, color4& color, rgeVector2& uv);
        void readVerticesFromRgm(std::istream& is);
        void renderTriangles();

        GLuint mDisplayList;
        GLuint mDisplayListNoMaterial;

        meshes mTriMeshes;
        rgeVector3s mPositions;

        SOURCE_TYPE mSource;

    private:
        int mCurrentMesh;
        //	bool mHasColors;
        //	bool mHasUV;

    };
    /*
    class QuadMeshObject : public TriMeshObject {
    public: 	
            QuadMeshObject(RGE* rge) : TriMeshObject(rge)
            {}
    //	static MeshObjectRef newInst() { return MeshObjectRef(new MeshObject()); }
            virtual void createFromRgm(std::istream& is) ;

            virtual void getTriangles(rgeVector3s& triangles);
            virtual void getTriangleIndexVertices(rgeVector3s& v, ints& index);
	
	
            virtual void clear() { mTriMeshes.clear(); mQuadMeshes.clear(); mVertices.clear(); mNormals.clear(); mColors.clear(); }

            virtual void getBoundingBox(rgeVector3& max, rgeVector3& min);	
            virtual void resolveMaterials() ;	
            virtual void render() ;
            virtual GEOM_TYPE geomType() { return MESH_OBJ; }
	
    private:
            meshes mQuadMeshes;
    };	
     typedef std::shared_ptr<QuadMeshObject> QuadMeshObjectRef;
     inline MeshObjectRef newQuadMeshObject(RGE* rge) { return QuadMeshObjectRef(new QuadMeshObject(rge)); }

     */

    typedef std::shared_ptr<MeshObject> MeshObjectRef;
    //inline MeshObjectRef newMeshObject(RGE* rge) { return MeshObjectRef(new MeshObject(rge)); }

    typedef std::shared_ptr<TriMeshObject> TriMeshObjectRef;

    inline TriMeshObjectRef newTriMeshObject() {
        return TriMeshObjectRef(new TriMeshObject);
    }

}
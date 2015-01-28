/*
 *  Renderable.h
 *  collada_test
 *
 *  Created by Yuhei Akamine on 07/06/02.
 *  Copyright 2007 Yuhei Akamine. All rights reserved.
 *
 */

#pragma once
#include <memory>

#include <vector>

#include "RGEObject.h"

namespace rge {
    
    class Renderable  {
    public:
        typedef enum {
            PRIMITIVE,
            TRI_MESH_OBJ,
            MESH_OBJ
        } GEOM_TYPE;
        
    private:
        GEOM_TYPE mGeomType;
        
    public:
        Renderable()  {}
        virtual ~Renderable() {}
        
        virtual void render() = 0;
        virtual void getBoundingBox(rgeVector3& max, rgeVector3& min) = 0;
        virtual GEOM_TYPE geomType() = 0;
    };
    
    class Issuable {
    public:
        typedef enum {
            CAMERA,
            MATERIAL,
            LIGHT,
        } OBJ_TYPE;
        
        Issuable() {}
        virtual ~Issuable() {}
        
        virtual void issue() = 0;
        virtual OBJ_TYPE objType() = 0;
    };
    
    typedef std::shared_ptr<Renderable> RenderableRef;
    typedef std::vector<RenderableRef> RenderableRefs; 
    typedef std::shared_ptr<Issuable> IssuableRef;
    typedef std::vector<IssuableRef> IssuableRefs; 
    
}
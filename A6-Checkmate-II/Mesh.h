#ifndef __MESH_H__
#define __MESH_H__

#include "Geometry.h"

//
// Representation for a polygon mesh model.
//
class Mesh {
public:
    // Make a new mesh, with mesh data populated from the given file.
    Mesh( char const *filename );
    
    // Destroy this mesh.
    virtual ~Mesh();
    
    /** Draw all the polygon faces in this mesh. */
    void draw();
    
private:
    GLfloat **vlist;
    GLfloat **nlist;
    GLushort **fvlist;
    GLushort **fnlist;
    int vNum, nNum, fNum;
};

#endif


//
// Mesh.h
//
// Assignment 6 - Checkmate II
//
// Author: Landon Epps
// Created: 12/5/2013
// Modified by: Landon Epps and Jesse Clary
// Modified on: 12/8/2013
//

#include "Mesh.h"
#ifdef __APPLE__
#include <glut/glut.h>
#else
#include "GL/glut.h"
#endif

#include <fstream>
#include <string>
#include <iostream>

using namespace std;

// Make a new mesh, with mesh data populated from the given file.
Mesh :: Mesh( char const *filename ) {
    // file stream to read in mesh
    ifstream meshFile;
    // attempt to open mesh
    meshFile.open(filename);
    
    // check if file failed to open
    if (!meshFile) {
        cerr << "invalid file name" << endl;
        exit(1);
    } else {
        // temp string for reading
        string temp;
        // ensure mesh has vlist
        if(meshFile >> temp && temp == "vlist") {
            // read in number of vertices
            meshFile >> vNum;
            // store the vertices in an array
            if (vNum > 0) {
                vlist = new GLfloat *[vNum];
                for (int i = 0; i < vNum; i++) {
                    vlist[i] = new GLfloat[3];
                    for (int j = 0; j < 3; j++) {
                        meshFile >> vlist[i][j];
                    }
                }
            }
        } else {
            cerr << "No vlist in mesh file.\n";
            exit(1);
        }
        // ensure mesh has nlist
        if (meshFile >> temp && temp == "nlist") {
            // read in number of normals
            meshFile >> nNum;
            // store the vertices in an array
            if (nNum > 0) {
                nlist = new GLfloat *[nNum];
                for (int i = 0; i < nNum; i++) {
                    nlist[i] = new GLfloat[3];
                    for (int j = 0; j < 3; j++) {
                        meshFile >> nlist[i][j];
                    }
                }
            }
        } else {
            cerr << "No nlist in mesh file.\n";
            exit(1);
        }
        // ensure mesh has flist
        if (meshFile >> temp && temp == "flist") {
            // read in number of faces
            meshFile >> fNum;
            if (fNum > 0) {
                // populate fnlist with the indices into the
                // nlist array
                // populate fvlist with the indices into the
                // vlist array
                fvlist = new GLushort *[fNum];
                fnlist = new GLushort *[fNum];
                
                for (int i = 0; i < fNum; i++) {
                    
                    meshFile >> temp;
                    int vNum = atoi(temp.c_str());
                    
                    fvlist[i] = new GLushort [vNum + 1];
                    fnlist[i] = new GLushort [vNum + 1];
                    fvlist[i][0] = fnlist[i][0] = vNum;
                    
                    if (vNum == 4 || vNum == 3) {
                        for (int j = 1; j <= vNum; j++) {
                            meshFile >> fvlist[i][j];
                            meshFile >> fnlist[i][j];
                        }
                    } else {
                        cerr << "Mesh invalid.\n";
                    }
                }
            }
        } else {
            cerr << "No flist in mesh file.\n";
            exit(1);
        }
        
        // close it all up
        meshFile.close();
    }
}

// Destroy this mesh.
Mesh :: ~Mesh() {
    // clear the allocated memory
    for (int i = 0; i < vNum; i++) {
        delete [] vlist[i];
    }
    for (int i = 0; i < nNum; i++) {
        delete [] nlist[i];
    }
    delete [] vlist;
    delete [] nlist;
    for (int i = 0; i < fNum; i++) {
        delete [] fvlist[i];
        delete [] fnlist[i];
    }
    delete [] fvlist;
    delete [] fnlist;
}

/** Draw all the polygon faces in this mesh. */
void Mesh :: draw() {
    
    // loop through all the faces,
    // manually setting the normal and vertex vectors
    for (int i = 0; i < fNum; i++) {
        glBegin(GL_POLYGON);
        for(int j = 1; j <= fvlist[i][0]; j++) {
            glNormal3fv(nlist[fnlist[i][j]]);
            glVertex3fv(vlist[fvlist[i][j]]);
        }
        glEnd();
    }
    
}

/************************************************
 Filename: Chess.cpp
 Author: Michael Poor
 Modified by: Jesse Clary and Landon Epps
 Modified: 8-Dec-2013
    - Implemented reflection of the pieces onto the
        board
 ***********************************************/
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <ctime>
#include <vector>
#include <algorithm>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#include <GL/glut.h>
#endif

#include "Geometry.h"
#include "Mesh.h"

using namespace std;

class ChessBoard {
    /* Different types of pieces, also, indices into meshList */
    enum PieceType { PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING };

    /** List of meshes, one for each piece type. */
    vector< Mesh * > meshList;

    /** Enum-hacked integer constants */
    enum { 
        /** Size of the board. */
        BOARD_SIZE = 8,
    };

    /** Record for an individual object in our scene. */
    struct Object {
        /** Transformation for the model. */
        Matrix trans;

        /** Color for the model */
        Vector color;
    
        /** Index in meshList of the mesh used to draw the model. */
        PieceType mesh;
    };

    /** List of objects in the scene. */
    vector< Object > objectList;

    /** Rotation angle for the view. */
    double camRotation;

    /** View elevation angle. */
    double camElevation;

    /** Mouse location for the last known mouse location, these are used
        for some of the mouse dragging operations. */
    int lastMouseX, lastMouseY;

    /** List of keys currently held down. */
    vector< int > dkeys;

    /** Index into objectList for the currently selected chess piece,
        or -1 if nothing is selected */
    int selection;

    /** Projection matrix for the current view. */
    Matrix projectionMatrix;

    /** Camera placement matrix for the current view. */
    Matrix cameraMatrix;

    /** Return true if the given key is being held down. */
    bool keyPressed( unsigned char key ) {
        return find( dkeys.begin(), dkeys.end(), key ) != dkeys.end();
    }

    /** Convenience function, return the value x, clamped to the [ low,
        high ] range. */
    static double clamp( double x, double low, double high ) {
        if ( x < low )
            x = low;
        if ( x > high )
            x = high;
        return x;
    }

    /** Utility function to fold camera placement into modelview matrix.
        Caller must pass in the aspect ratio of the viewport.  If
        wipeProjection is true, clear out the projection matrix before
        installing the projection.  This is an accommodationn to the
        object selection code (which I've removed for now).  */
    void placeCamera( double aspect, bool wipeProjection = true ) {
        // Set up a perspective projection based on the window aspect.
        glMatrixMode( GL_PROJECTION );
        if ( wipeProjection )
            glLoadIdentity();
        
        // projection matrix, based on the matrix in OpenGL's
        // documentation for gluPerspective
        float tmpProjMatrix[16] = {2/(float)aspect, 0, 0, 0,
            0, 2, 0, 0,
            0, 0, -1, -1,
            0, 0, -8.2, 0};
        
        // set the projection matrix
        projectionMatrix = Matrix::glConvert(tmpProjMatrix);
        projectionMatrix.glMult();
        
        // half of the board size
        double halfBoard = BOARD_SIZE / 2.0;
        
        // move camera to center of board, rotate about the Y axis,
        // rotate about the X axis, translate back along the Z axis
        cameraMatrix = Matrix::identity();
        cameraMatrix = cameraMatrix * Matrix::translate(0, 0, -12);
        cameraMatrix = cameraMatrix * Matrix::rotateX(camElevation);
        cameraMatrix = cameraMatrix * Matrix::rotateY(camRotation);
        cameraMatrix = cameraMatrix * Matrix::translate(-halfBoard, 0, -halfBoard);
        
        // set OpenGl's MODELVIEW matrix to cameraMatrix
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        cameraMatrix.glMult();
    }

    void drawScene() {
        // Don't use z-buffer while we draw the board and shadows.
        glDisable( GL_DEPTH_TEST );
        
        // enable blending for shadows and reflections
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        
        //
        // create a stencil for the board
        //
        glEnable(GL_STENCIL_TEST);
        
        glStencilMask(0xFF);
        glClear(GL_STENCIL_BUFFER_BIT);
        glStencilFunc(GL_ALWAYS, 1, 1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        
        glBegin( GL_QUADS );
        for ( int x = 0; x < BOARD_SIZE; x++ )
            for ( int z = 0; z < BOARD_SIZE; z++ ) {
                // Pick a color based on the parity of the square.
                if ( ( x + z ) % 2 == 0 )
                    glColor3f( 0.8, 0.6, 0.3 );
                else
                    glColor3f( 0.9, 0.4, 0.3 );
                
                // Draw a 1x1 quad for this square.
                glVertex3d( x, 0, z );
                glVertex3d( x, 0, z + 1 );
                glVertex3d( x + 1, 0, z + 1 );
                glVertex3d( x + 1, 0, z );
            }
        glEnd();
        
        glStencilFunc(GL_EQUAL, 1, 1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        // Enable clipping plane to clip the reflections and shadows.
        glEnable(GL_CLIP_PLANE0);
        
        //
        // Draw reflections
        //
        glEnable( GL_DEPTH_TEST );
        for ( int i = 0; i < objectList.size(); i++ ) {
            // Get the object into a local variable, for convenience.
            Object &obj = objectList[ i ];
            
            Vector scaledColor = obj.color;
            
            // if the current chess piece is selected
            if (i == selection) {
                scaledColor = scaledColor * 1.5;
            }
            
            glColor4f(scaledColor.x, scaledColor.y, scaledColor.z, 0.50);
            
            // Apply the object's transformation.
            glPushMatrix();
            
            // apply the shadow matrix, flattening the y axis
            Matrix trans = Matrix::identity();
            trans = trans * obj.trans;
            trans = trans * Matrix::rotateZ(180);
            trans.glMult();
            
            //
            // Draw the mesh.
            meshList[ objectList[ i ].mesh ]->draw();
            
            glPopMatrix();
        }
        
        //
        // Draw shadows
        //
        glDisable(GL_DEPTH_TEST);
        for ( int i = 0; i < objectList.size(); i++ ) {
            // Get the object into a local variable, for convenience.
            Object &obj = objectList[ i ];
            
            // set the shadow color to transparent black
            glColor4f(0, 0, 0, 0.50);
            
            // Matrix to create shadows
            float shadowMatrix[16] = {18, 0, 0, 0,
                -1, 18, -1, -1,
                0, 0, 18, 0,
                0, 0, 0, 18};
            
            
            // Apply the object's transformation.
            glPushMatrix();
            
            // apply the shadow matrix, flattening the y axis
            Matrix trans = Matrix::scale(1, 0, 1);
            trans = trans * Matrix::glConvert(shadowMatrix);
            trans = trans * obj.trans;
            trans.glMult();
            
            //
            // Draw the mesh.
            meshList[ objectList[ i ].mesh ]->draw();
            
            glPopMatrix();
        }

        // Done drawing shadows/reflection; disable blending
        glDisable( GL_BLEND );

        // Use Z-buffer while we draw the playing pieces.
        glEnable( GL_DEPTH_TEST );
        
        // set the material properties of the pieces
        // to have nice specual highlights
        GLfloat mat_specular[] = { 0.5, 0.5, 0.5, 1.0 };
        GLfloat mat_shininess[] = { 50.0 };
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
        // disable clipping plane and stencil test before drawing
        // the pieces that we don't want clipped by the stencil.
        glDisable(GL_CLIP_PLANE0);
        glDisable(GL_STENCIL_TEST);
        // Draw everything on the board.
        for ( int i = 0; i < objectList.size(); i++ ) {
            // Get the object into a local variable, for convenience.
            Object &obj = objectList[ i ];
            
            Vector scaledColor = obj.color;
            
            // if the current chess piece is selected
            if (i == selection) {
                scaledColor = scaledColor * 1.5;
            }
            
            glColor3f(scaledColor.x, scaledColor.y, scaledColor.z);

            // Apply the object's transformation.
            glPushMatrix();
            obj.trans.glMult();
            
            // Add name to the namestack
            glPushName(i);

            // Draw the mesh.
            meshList[ objectList[ i ].mesh ]->draw();
            
            glPopName();
      
            glPopMatrix();
        }
        // reset the material properties for shadows and the board
        // to remove specular highlights
        GLfloat mat_specular_zero[] = { 0, 0, 0, 1.0 };
        GLfloat mat_shininess_zero[] = { 0 };
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular_zero);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess_zero);
    }

    /** Find any geometry that's near the mouse x, y location,
        and return a copy of the namestack for the closest object
        in depth at that location.  If no object is found, an empty
        vector is returned. */
    vector< GLuint > selectGeometry( int x, int y ) {
        // Get the window size for setting up the camera.
        int winWidth = glutGet( GLUT_WINDOW_WIDTH );
        int winHeight = glutGet( GLUT_WINDOW_HEIGHT );

        // Get a copy of the viewport transformation.
        GLint view[ 4 ];
        glGetIntegerv( GL_VIEWPORT, view );
        
        // Set up a tiny little view volume to help with selection, put the
        // pick matrix transformation after the projection.
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        gluPickMatrix( x, winHeight - y, 3, 3, view );
        placeCamera( double( winWidth ) / winHeight, false );

        // Perpare for drawing in selection mode.
        GLuint buffer[ 2048 ];
        glSelectBuffer( sizeof( buffer ) / sizeof( buffer[ 0 ] ), buffer );
        glRenderMode( GL_SELECT );
        
        // Draw the whole scene, and see what hits the pick volume
        drawScene();
        glFlush();
        
        // Simplified list of all hit records, this is kind of stupid since
        // we don't need all this information.  However, it does make it
        // easy to extract the closest record at the end.
        vector< pair< GLuint, vector< GLuint > > > snapshot;

        // See if we hit anything.
        int hcount = glRenderMode( GL_RENDER );
        if ( hcount == -1 ) {
            cerr << "Selection buffer overflow!" << endl;
        } else {
            // Find the closest object that was hit by inspecting
            // the list of hit records built duing rasterization.
            int pos = 0;
            while ( hcount ) {
                // Make a copy of this name stack report.
                pair< GLuint, vector< GLuint > > rec;
                rec.first = buffer[ pos + 1 ];
                for ( int i = 0; i < buffer[ pos ]; i++ )
                    rec.second.push_back( buffer[ pos + 3 + i ] );

                // Put the new record on the list of records.
                snapshot.push_back( rec );
            
                // Move to the next item in the selection buffer.
                pos += 3 +  buffer[ pos ];
                hcount--;
            }
        }

        // Find the closest hit record, and return it if there is one.
        sort( snapshot.begin(), snapshot.end() );
        if ( snapshot.size() == 0 )
            return vector< GLuint >();
        return snapshot.front().second;
    }

public:
    ~ChessBoard() {
        // Delete all the meshes we loaded.
        while ( meshList.size() ) {
            delete meshList.back();
            meshList.pop_back();
        }
    }

    /** Create output window, initialize OpenGL features for the driver. */
    void init( int &argc, char *argv[] ) {
        // Load peshes for all the pieces.
        meshList.push_back( new Mesh( "pawn.mesh" ) );
        meshList.push_back( new Mesh( "rook.mesh" ) );
        meshList.push_back( new Mesh( "knight.mesh" ) );
        meshList.push_back( new Mesh( "bishop.mesh" ) );
        meshList.push_back( new Mesh( "queen.mesh" ) );
        meshList.push_back( new Mesh( "king.mesh" ) );

        // Make a new window with double buffering and with Z buffer.
        glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL );
        glutInitWindowSize( 800, 600 );
        glutCreateWindow( "Chess Board" );
    
        // Initialize background color.
        glClearColor( 0.6, 0.6, 0.6, 0 );
        
        // enable smoothing
        glShadeModel(GL_SMOOTH);
        // enable colored shading
        glEnable(GL_COLOR_MATERIAL);
        // enable lighting
        glEnable(GL_LIGHTING);
        // enable one light
        glEnable(GL_LIGHT0);
        
        // position of light
        GLfloat light0_pos[] = {1.0, 18.0, 1.0, 1.0};
        // anbient component
        GLfloat ambient0[] = {0.2, 0.2, 0.2, 1.0};
        // diffuse component
        GLfloat diffuse0[] = {0.5, 0.5, 0.5, 1.0};
        
        // set light0's properties
        glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);

        // Place all the pieces on the chess board.  This should probably
        // be driven by a data file, rather than hard-coded.
        {
            // Place all the dark pawns.
            Object example;
            example.color = Vector( 1, 0.3, 0.3 );
            example.mesh = PAWN;
            example.trans = Matrix::translate( 0.5, 0, 1.5 );
            objectList.push_back( example );
            example.trans = Matrix::translate( 1.5, 0, 1.5 );
            objectList.push_back( example );
            example.trans = Matrix::translate( 2.5, 0, 1.5 );
            objectList.push_back( example );
            example.trans = Matrix::translate( 3.5, 0, 1.5 );
            objectList.push_back( example );
            example.trans = Matrix::translate( 4.5, 0, 1.5 );
            objectList.push_back( example );
            example.trans = Matrix::translate( 5.5, 0, 1.5 );
            objectList.push_back( example );
            example.trans = Matrix::translate( 6.5, 0, 1.5 );
            objectList.push_back( example );
            example.trans = Matrix::translate( 7.5, 0, 1.5 );
            objectList.push_back( example );

            // Place the two dark rooks.
            example.mesh = ROOK;
            example.trans = Matrix::translate( 0.5, 0, 0.5 );
            objectList.push_back( example );
            example.trans = Matrix::translate( 7.5, 0, 0.5 );
            objectList.push_back( example );

            // Place the two dark knights.
            example.mesh = KNIGHT;
            example.trans = Matrix::translate( 1.5, 0, 0.5 );
            objectList.push_back( example );
            example.trans = Matrix::translate( 6.5, 0, 0.5 );
            objectList.push_back( example );

            // Place the two dark bishops.
            example.mesh = BISHOP;
            example.trans = Matrix::translate( 2.5, 0, 0.5 );
            objectList.push_back( example );
            example.trans = Matrix::translate( 5.5, 0, 0.5 );
            objectList.push_back( example );

            // Place the dark queen.
            example.mesh = QUEEN;
            example.trans = Matrix::translate( 3.5, 0, 0.5 );
            objectList.push_back( example );

            // Place the dark king.
            example.mesh = KING;
            example.trans = Matrix::translate( 4.5, 0, 0.5 );
            objectList.push_back( example );

            // Make the other side as copies of all the pieces, with different
            // colors and rotated around the center of the board
            for ( int i = 0; i < 16; i++ ) {
                example = objectList[ i ];
                example.color = Vector( 0.7, 0.7, 0.4 );
                example.trans = Matrix::translate( 4, 0, 4 ) *
                    Matrix::rotateY( 180 ) *
                    Matrix::translate( -4, 0, -4 ) *
                    example.trans;
                objectList.push_back( example );
            }

            // Queens actually need to be facing each other, so swap light
            // king and queen.
            swap( (objectList.end() - 1)->trans, (objectList.end() - 2)->trans );
        }

        // Set initial camera configuration
        camRotation = 0;
        camElevation = 30;

        // Nothing is selected yet.
        selection = -1;
    
    }

    /** Redraw the contetns of the display */  
    void display() {
        // Figure out the aspect ratio.
        int winWidth = glutGet( GLUT_WINDOW_WIDTH );
        int winHeight = glutGet( GLUT_WINDOW_HEIGHT );

        // Make sure we take camera position into account.
        placeCamera( double( winWidth ) / winHeight );

        // Clear the color and the Z-Buffer components.
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        // Draw everything
        drawScene();

        // Show it to the user.
        glutSwapBuffers();
    }

    /** Callback for key down events */
    void keyDown( unsigned char key, int x, int y ) {
        // Add key to the pressed list if it's not already there.
        if ( !keyPressed( key ) )
            dkeys.push_back( key );

        // Remember where the mouse was when this key was pressed.
        lastMouseX = x;
        lastMouseY = y;
    }

    /** Callback for key down events */
    void keyUp( unsigned char key, int x, int y ) {
        // Remove key from the list of down keys, if it's in there.
        vector< int > :: iterator pos;
        while ( ( pos = find( dkeys.begin(), dkeys.end(), key ) ) != dkeys.end() )
            dkeys.erase( pos );

        // Remember where the mouse was when this key was released.
        lastMouseX = x;
        lastMouseY = y;
    }

    /** Callback for when the mouse button is pressed or released */
    void mouse( int button, int state, int x, int y ) {
        if (button == GLUT_LEFT_BUTTON) {
            vector<GLuint> namestack = selectGeometry(x, y);
            if (namestack.size() > 0) {
                selection = namestack[0];
            } else {
                selection = -1;
            }
        }
        glutPostRedisplay();
    }

    /** Callback for when the user moves the mouse with a button pressed. */
    void motion( int x, int y ) {
    }

    /** Callback for when the mouse is moved without a button being pressed. */
    void passiveMotion( int x, int y ) {
        // If 'a' is being held down, move the camera around.
        if ( keyPressed( 'a' ) ) {
            // Rotate the camera when a is pressed (for angle)
            camElevation = clamp( camElevation + ( y - lastMouseY ) / 2.0, 10, 80 );
            camRotation = camRotation + ( x - lastMouseX ) / 2.0;

            glutPostRedisplay();
        }

        // Snapshot the new mouse location, subsequent moves are handled
        // incrementally.
        lastMouseX = x;
        lastMouseY = y;
    }
};

// One static instance of our chess object.
static ChessBoard chessBoard;

// Stub function to intercept the display callback
//
void display() {
    chessBoard.display();
}

// Callback for when keys are pressed down.
void keyDown( unsigned char key, int x, int y ) {
    chessBoard.keyDown( key, x, y );
}

// Callback for when keys are released
void keyUp( unsigned char key, int x, int y ) {
    chessBoard.keyUp( key, x, y );
}

// Callback for when the mouse button is pressed.
void mouse( int button, int state, int x, int y ) {
    chessBoard.mouse( button, state, x, y );
}

// Callback for when the mouse is moved with a button pressed.
void motion( int x, int y ) {
    chessBoard.motion( x, y );
}

// Callback for when the mouse is moved without a button pressed.
void passiveMotion( int x, int y ) {
    chessBoard.passiveMotion( x, y );
}

/////////////////////////////////////////////////////////////////
// Glut callback functions.
/////////////////////////////////////////////////////////////////

int main( int argc, char **argv ) {
    // Init glut and make a window.
    glutInit( &argc, argv );

    chessBoard.init( argc, argv );

    // Register all our callbacks.
    glutDisplayFunc( display );
    glutIgnoreKeyRepeat( true );
    glutKeyboardFunc( keyDown );
    glutKeyboardUpFunc( keyUp );
    glutMouseFunc( mouse );
    glutMotionFunc( motion );
    glutPassiveMotionFunc( passiveMotion );

    // Let glut handle UI events.
    glutMainLoop();

    return 0;
}


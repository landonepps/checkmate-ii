#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <math.h>
#include <iostream>
#include <cstdlib>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#include <GL/glut.h>
#endif

const double PI = acos(-1.0);

/**
   Geometry utility types and operations to help with common 3D graphics
   tasks.
*/

// Forward declarations for free functions used in the class.
struct Vector;
inline Vector operator/( Vector const &a, double b );

/**
   A 3D vector/point in homogeneous coordinates.  Defined as a struct
   so its data members can be directly manipulated by client code.
   Most member functions are defined here so that they can be inlined.
   The name, Vector, is used so it won't conflict with the STL vector.
*/
struct Vector {
    /**
       Make an uninitialized vector.
    */
    Vector() {
        x = 0;
        y = 0;
        z = 0;
        w = 0;
    }

    /**
       Make a 3D vector in homogeneous coordinates (last component is zero)
    */
    Vector( double xv, double yv, double zv ) {
        x = xv;
        y = yv;
        z = zv;
        w = 0;
    }

    /**
       Make a 3D vector/point in homogeneous coordinates (last component is 
       supplied by the caller)
    */
    Vector( double xv, double yv, double zv, double wv ) {
        x = xv;
        y = yv;
        z = zv;
        w = wv;
    }

    /**
       Add the given vector's components to this one.
    */
    void operator+=( Vector const &a ) {
        x += a.x;
        y += a.y;
        z += a.z;
        w += a.w;
    }

    /**
       Subtract the given vector's components to this one.
    */
    void operator-=( Vector const &a ) {
        x -= a.x;
        y -= a.y;
        z -= a.z;
        w -= a.w;
    }

    /**
       Multiply each component of this vector by the given scalar.
    */
    void operator*=( double s ) {
        x *= s;
        y *= s;
        z *= s;
        w *= s;
    }

    /**
       Divide each component of this vector by the given scalar.
    */
    void operator/=( double s ) {
        x /= s;
        y /= s;
        z /= s;
        w /= s;
    }

    /**
       Return the cross product of this vector with the given vector.
       Only meaningful for vectors (not points)
    */
    Vector cross( Vector const &b ) const {
        return Vector( y * b.z - z * b.y,
                       z * b.x - x * b.z,
                       x * b.y - y * b.x );
    }

    /**
       Return the squared magnitude of this vector.  This is only
       meaningful for vectors (not points)
    */
    double magSquared() const {
        return x * x + y * y + z * z;
    }

    /**
       Return the magnitude of this vector.   This is only
       meaningful for vectors (not points).
    */
    double mag() const {
        return sqrt( magSquared() );
    }

    /**
       Return a normalized version of this vector.   This is only
       meaningful for vectors with non-zero length (not points).
    */
    Vector norm() const {
        return *this / mag();
    }

    /**
       Get OpenGL to draw the contents of this vector as a vertex.
       This assumes that the vector actually contains a point.
    */
    void glVertex() const {
        glVertex4d( x, y, z, w );
    }

    /**
       Get OpenGL to draw the contents of this vector as a normal.
    */
    void glNormal() const {
        glNormal3d( x, y, z );
    }

    /**
       Get OpenGL to draw the contents of this vector as a color.
    */
    void glColor() const {
        glColor3f( x, y, z );
    }

    /**
       Representation of the Vector.
    */
    double x, y, z, w;
};

/**
   Overloaded operator for vector vector addition.
   Return a vector that is the sum of the given two vectors.
*/
inline Vector operator+( Vector const &a, Vector const &b ) {
    return Vector( a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w );
}

/**
   Overloaded operator for vector vector subtraction.
   Return a vector that is the difference of the given two vectors.
*/
inline Vector operator-( Vector const &a, Vector const &b ) {
    return Vector( a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w );
}

/**
   Overloaded operator for vector vector multiplication computes the dot
   product.  Since dot product is only meaningful for actual vectors
   (not points) the 4th component is ignored.
*/
inline double operator*( Vector const &a, Vector const &b ) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/**
   Overloaded operator for scalar vector multiplication.  Returns the
   result in a vector.  Generally, meaningful for vector and points when they
   are part of an affine sum.
*/
inline Vector operator*( double a, Vector const &b ) {
    return Vector( a * b.x, a * b.y, a * b.z, a * b.w );
}

/**
   Overloaded operator for vector scalar multiplication.  Returns the
   result in a vector.  Generally, meaningful for vector and points
   when they are part of an affine sum.
*/
inline Vector operator*( Vector const &a, double b ) {
    return Vector( a.x * b, a.y * b, a.z * b, a.w * b );
}

/**
   Return a new vector that is this vector divided by the given
   scalar.  Generally, meaningful for vector and points when they are
   part of an affine sum.
*/
inline Vector operator/( Vector const &a, double b ) {
    return Vector( a.x / b, a.y / b, a.z / b, a.w / b );
}

/**
   Convenience function to print out the contents of a Vector.
*/
std::ostream &operator<<( std::ostream &stream, Vector const &v );

/**
   Implementation of a 4x4 matrix, typically used for affine
   transformations and projections.
*/
class Matrix {
 public:
    /**
       Make the an uninitialized matrix.
    */
    Matrix() {
    }

    /**
       Return a new matrix that is the transpose of this one.
    */
    Matrix transpose() const;

    /**
       Return the inverse of this matrix.  Behavior is undefined if the matrix
       is singular.
    */
    Matrix inverse() const;

    /**
       Return the requested row of the matrix (as an array of doubles)
       This is useful since an extra pair of [] after this will give
       a particular element (e.g. m[1][3]
    */
    double *operator[]( int ind ) {
        return val[ ind ];
    }

    /**
       For a constant matrix, return an unmodifiable row of the matrix.
    */
    double const *operator[]( int ind ) const {
        return val[ ind ];
    }

    /**
       Return a copy of the identity matrix.
    */
    static Matrix identity();

    /**
       Return a translation matrix that translates by the given displacement
       on each of axis.
    */
    static Matrix translate( double dx, double dy, double dz );

    /**
       Return a scaling matrix that scales by the given factors on each axis.
    */
    static Matrix scale( double sx, double sy, double sz );

    /**
       Return a rotation matrix that rotates by the given angle (in degrees)
       on the X axis.
    */
    static Matrix rotateX( double angle );

    /**
       Return a rotation matrix that rotates by the given angle (in degrees)
       on the Y axis.
    */
    static Matrix rotateY( double angle );

    /**
       Return a rotation matrix that rotates by the given angle (in degrees)
       on the Z axis.
    */
    static Matrix rotateZ( double angle );

    /**
       Return a new matrix corresponding to the given matrix
       of floating point values given in column major order.  This
       makes it easy to go from the OpenGL representation of a 
       matrix to an instance of Matrix. */
    static Matrix glConvert( float mat[] );

    /**
       Make this matrix a conversion from the grame with the given
       x, y and z axes and origin to the frame in which the coordinates for
       they are given.
    */
    static Matrix frame( Vector const &vx, Vector const &vy, 
                         Vector const &vz, Vector const &o );

    /**
       Postmultiply this matrix by the current OpenGL transformation matrix.
    */
    void glMult() const;

 private:
    /** representation for the contents of the matrix */
    double val[ 4 ][ 4 ];
};

/**
   Return the product of the matrix, m and the given vector, v.
*/
inline Vector operator*( Matrix const &m, Vector const &v ) {
    return Vector(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w,
                  m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
                  m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
                  m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w);
}

/**
   Return the product of the two given matrices.
*/
Matrix operator*( Matrix const &a, Matrix const &b );

/**
   Convenience function to print out the contents of a Vector.
*/
std::ostream &operator<<( std::ostream &s, Matrix const &m );

#endif

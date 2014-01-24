#include "Geometry.h"
#include <algorithm>
#include <iomanip>

using namespace std;

// Implementation of geometry functions that are two big or not sufficiently
// performance critical to go in the header file.

ostream &operator<<( ostream &stream, Vector const &v ) {
    // Just print out the components of the vector.
    stream << "[ " << v.x << ", " << v.y << ", " << v.z << ", " << v.w << " ]";
    return stream;
}

/////////////////////////////////////////////////////////////////////////////
// Matrix
/////////////////////////////////////////////////////////////////////////////

Matrix Matrix :: transpose() const {
    Matrix result;

    // Copy every value from the input matrix to the output result, flipping
    // across the diagonal as we go.
    for( int r = 0; r < 4; r++ )
        for( int c = 0; c < 4; c++ )
            result.val[ r ][ c ] = val[ c ][ r ];
    
    return result;
}

Matrix Matrix :: inverse() const {
    Matrix result = Matrix :: identity();
    Matrix m = *this;

    int r, c, i;

    // Try to make m into the identity matrix.  Perform corresponding
    // operations on result;
    for ( r = 0; r < 4; r++ ) {
        int pr = r;
        // Find the best pivot (i.e. largest magnitude)
        for ( i = r + 1; i < 4; i++ )
            if ( fabs( m[ i ][ r ] ) > fabs( m[ pr ][ r ] ) )
                pr = i;

        // Swap rows to put the best pivot on the diagonal.
        for ( i = 0; i < 4; i++ ) {
            swap( m[ r ][ i ], m[ pr ][ i ] );
            swap( result[ r ][ i ], result[ pr ][ i ] );
        }

        // Normalize the pivot row (i.e. put a one on the diagonal)
        double pivot = m[ r ][ r ];
        for ( c = 0; c < 4; c++ ) {
            m[ r ][ c ] /= pivot;
            result[ r ][ c ] /= pivot;
        }

        // Introduce zeros above and below the pivot.
        for ( i = 0; i < 4; i++ )
            if ( i != r ) {
                double scale = m[ i ][ r ];
                for ( c = 0; c < 4; c++ ) {
                    m[ i ][ c ] -= scale * m[ r ][ c ] ;
                    result[ i ][ c ] -= scale * result[ r ][ c ] ;
                }
            }
    }

    return result;
}

Matrix Matrix :: identity() {
    Matrix result;

    for( int r = 0; r < 4; r++ )
        for( int c = 0; c < 4; c++ )
            // Store a 1 iff we are on the diagonal.
            if( r == c )
                result[ r ][ c ] = 1;
            else
                result[ r ][ c ] = 0;

    return result;
}

Matrix Matrix :: translate( double dx, double dy, double dz ) {
    // Make the matrix as a modification from the identity.
    Matrix result = identity();
    result.val[ 0 ][ 3 ] = dx;
    result.val[ 1 ][ 3 ] = dy;
    result.val[ 2 ][ 3 ] = dz;

    return result;
}

Matrix Matrix :: scale( double sx, double sy, double sz ) {
    // Make the matrix as a modification from the identity.
    Matrix result = identity();
    result.val[ 0 ][ 0 ] = sx;
    result.val[ 1 ][ 1 ] = sy;
    result.val[ 2 ][ 2 ] = sz;
    
    return result;
}

Matrix Matrix :: rotateX( double angle ) {
    // Make the matrix as a modification from the identity.
    Matrix result = identity();
    double c = cos( angle / 180 * PI );
    double s = sin( angle / 180 * PI );
    result.val[ 1 ][ 1 ] = c;
    result.val[ 1 ][ 2 ] = -s;
    result.val[ 2 ][ 1 ] = s;
    result.val[ 2 ][ 2 ] = c;

    return result;
}

Matrix Matrix :: rotateY( double angle ) {
    // Make the matrix as a modification from the identity.
    Matrix result = identity();
    double c = cos( angle / 180 * PI );
    double s = sin( angle / 180 * PI );
    result.val[ 0 ][ 0 ] = c;
    result.val[ 0 ][ 2 ] = s;
    result.val[ 2 ][ 0 ] = -s;
    result.val[ 2 ][ 2 ] = c;

    return result;
}

Matrix Matrix :: rotateZ( double angle ) {
    // Make the matrix as a modification from the identity.
    Matrix result = identity();
    double c = cos( angle / 180 * PI );
    double s = sin( angle / 180 * PI );
    result.val[ 0 ][ 0 ] = c;
    result.val[ 0 ][ 1 ] = -s;
    result.val[ 1 ][ 0 ] = s;
    result.val[ 1 ][ 1 ] = c;

    return result;
}

Matrix Matrix :: frame( Vector const &vx, Vector const &vy, 
                        Vector const &vz, Vector const &o ) {
    Matrix result;
    result.val[ 0 ][ 0 ] = vx.x;
    result.val[ 1 ][ 0 ] = vx.y;
    result.val[ 2 ][ 0 ] = vx.z;
    result.val[ 3 ][ 0 ] = 0;

    result.val[ 0 ][ 1 ] = vy.x;
    result.val[ 1 ][ 1 ] = vy.y;
    result.val[ 2 ][ 1 ] = vy.z;
    result.val[ 3 ][ 1 ] = 0;

    result.val[ 0 ][ 2 ] = vz.x;
    result.val[ 1 ][ 2 ] = vz.y;
    result.val[ 2 ][ 2 ] = vz.z;
    result.val[ 3 ][ 2 ] = 0;

    result.val[ 0 ][ 3 ] = o.x;
    result.val[ 1 ][ 3 ] = o.y;
    result.val[ 2 ][ 3 ] = o.z;
    result.val[ 3 ][ 3 ] = 1;

    return result;
}

Matrix Matrix :: glConvert( float mat[] ) {
    // Storage for the new matrix.
    Matrix m;
  
    // Convert from column major order.
    for( int r = 0; r < 4; r++ )
        for( int c = 0; c < 4; c++ )
            m[ r ][ c ] = mat[ r + c * 4 ];
  
    // Return the matrix we just made.
    return m;
}

void Matrix :: glMult() const {
    // Storage for OpenGL matrix.
    GLdouble mat[ 16 ];
  
    // Populate mat from this matrix.
    for( int r = 0; r < 4; r++ )
        for( int c = 0; c < 4; c++ )
            mat[ r + c * 4 ] = val[ r ][ c ];
  
    // Append to OpenGL current transformation.
    glMultMatrixd( mat );
}

Matrix operator*( Matrix const &a, Matrix const &b ) {
    Matrix result;

    // For each cell of the result matrix.
    for( int r = 0; r < 4; r++ )
        for( int c = 0; c < 4; c++ ){
            // Compute the dot product of a row of a and a column of b.
            double sum = 0;
            for( int i = 0; i < 4; i++ )
                sum += a[ r ][ i ] * b[ i ][ c ];
            result[ r ][ c ] = sum;
        }
  
    return result;
}

ostream &operator<<( ostream &s, Matrix const &m ) {
    s.setf( ios::fixed );
    s << setprecision( 4 );
    // Use Setw for each cell so the matrix has a better chance of lining up.
    for ( int r = 0; r < 4; r++ )
        s << "| " << setw( 10 ) << m[ r ][ 0 ] << " "
          << setw( 10 ) << m[ r ][ 1 ] << " "
          << setw( 10 ) << m[ r ][ 2 ] << " "
          << setw( 10 ) << m[ r ][ 3 ] << " |" << endl;
    return s;
}

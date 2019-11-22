#define MAX_COST_PM 10e6
#define MAX_COST_NCC 2.0f
#define MAX_COST_CENSUS 10e4
#define MAX_COST_BILATERAL_NCC 2.0f

#define WINDOW_INCREMENT 2
#define WINDOW_SIZE 15

/**
* @brief Compute projection from one image to the other using given homography
* @param p Position in image 1
* @param H homography
* @return Position in image 2
* @note If position is invalid (depth is zero), return (-1,-1)
*/
int2 pixel_position( const int2 p,
                     const float H[9] ) ;

/**
 * @brief Compute projection from one image to the other using given homography
 * @param p Input position in image 1
 * @param h1x First row of the homography
 * @param h2x Second row of the homography
 * @param h3x Third row of the homography
 Assuming H =
 {
  ( H[0] , H[1] , H[2] , 0.0 )
  ( H[3] , H[4] , H[5] , 0.0 )
  ( H[6] , H[7] , H[8] , 0.0 )
 }
*/
int2 pixel_position2( const int2 p,
                      const float4 h1x,
                      const float4 h2x,
                      const float4 h3x ) ;

                      /**
* @brief Matrix multiplication between two matrices
* @param a A 3x3 matrix
* @param b A 3x3 matrix
* @param[out] A 3x3 matrix, result of the product of a and b
*/
void mat_mul_33( const float * a, const float * b,
                 float * out ) ;

/**
* @brief Scale a matrix by a scalar factor
* @param[in,out] out 3x3 matrix to scale
* @param factor Scaling factor
*/
void mat_scale_in_33( float * out, const float factor ) ;

/**
* @brief Cwise addition of two matrix
* @param a A 3x3 matrix
* @param b A 3x3 matrix
* @param[out] out Result of a+b
*/
void mat_add_33( const float * a, const float * b, float * out ) ;

/**
* @brief Cwise subtraction of two matrix
* @param a A 3x3 matrix
* @param b A 3x3 matrix
* @param[out] out Result of a-b
*/
void mat_sub_33( const float * a, const float * b, float * out ) ;

/**
* @brief Matrix multiplication of two vectors
* @param a A Column vector
* @param b a Row vector
* @param[out] Result of matrix multiplication of a and b
*/
void mat_outer_product_33( const float * a, const float * b, float * out ) ;

/**
* @brief Compute homography given a stereo rig and an a plane
* @param R Rotation from first view to the second
* @param t Translation from first view to the second
* @param Kref_inv Inverse intrinsic matrix of the first view
* @param Kother Intrinsic matrix of the second view
* @param plane_n Normal of the plane
* @param plane_d Parameter of the plane (dot(Pt,N) for a point Pt on the plane)
* @param[out] Resulting homography
*/
void compute_homography( const float * R,
                         const float * t,
                         const float * Kref_inv,
                         const float * Kother,
                         const float * plane_n,
                         const float plane_d,
                         float *H  ) ;

/**
* @param Sort an array
* @param[in,out] array The array to sort
* @param max_nb_elt Number of element in the array
*/
void sort_array( float * array, const int max_nb_elt ) ;

/**
* @brief Given a normalized direction, computes two vectors to form an orthonormalized frame
* @param n Input direction
* @param x Output x direction
* @param y Output y direction
*/
void ComputeOrthoNormalizedFrame( const float4 n,
                                  float4 * x,
                                  float4 * y ) ;


/**
* @brief Compute plane parameter d using depth, position and normal
* @param camP Projection matrix (3x4)
* @param camMinv Inverse rotational part of the projection matrix (3x3)
* @param n Plane normal
* @param pos Pixel position
* @param depth Depth at which intersection is made
* @note this computes intersection between the ray from position to the plane at specified depth
*/
float compute_plane_d( const float * camP,
                       const float * camMinv,
                       const float4 n,
                       const int2 pos,
                       const float depth );

/**
* @brief Convert from depth value to disparity value
* @param d Depth/disparity
* @param fx Focal in pixel
* @param baseline
* @return Disparity/Depth
*/
float convert_depth_disparity( const float d, const float fx, const float baseline ) ;

/**
* @brief Get a view vector (from origin to the scene) passing through a pixel
* @param camP Camera projection matrix (3x4)
* @param camMinv Camera inverse of rotational part (3x3)
* @param camCenter Position of the camera (3x1)
* @param pos Pixel position
* @return Normalized direction
*/
float4 compute_view_direction( const float * camP, const float * camMinv, const float4 camCenter, const int2 pos ) ;


/**
* @brief Sample a direction in a cone centered around Z axis
* @param u1 random value in range [0;1]
* @param u2 random value in range [0;1]
* @param cos_theta_max Cosinus of the maximum angle between generated direction and Z axis
* @return Sampled direction
*/
float4 UniformSampleCone( const float u1, const float u2, const float cos_theta_max ) ;

/**
* @brief Sample a direction in a cone centered around specified direction
* @param u1 random value in range [0;1]
* @param u2 random value in range [0;1]
* @param cos_theta_max Cosinus of the maximum angle between generated direction and specified direction
* @param n specified direction
* @return sampled direction
*/
float4 UniformSampleConeAroundNormal( const float u1, const float u2, const float cos_theta_max, const float4 n ) ;



/**
* @brief Compute pixel position given an homography
* @param p point Position in 2d
* @param H homography (3x3 matrix)
* @return related point in 2d
*/
int2 pixel_position( const int2 p,
                     const float H[9] )
{
  const float x = ( float ) p.x ;
  const float y = ( float ) p.y ;

  const float qx = H[0] * x + H[1] * y + H[2] ;
  const float qy = H[3] * x + H[4] * y + H[5] ;
  const float qz = H[6] * x + H[7] * y + H[8] ;

  if( fabs( qz ) <= 0.00001f )
  {
    return ( int2 ) ( -1, -1 ) ;
  }
  return ( int2 )( floor( qx / qz ), floor( qy / qz ) ) ;
}

/* Assuming H =
 {
  h1x = ( H[0] , H[1] , H[2] , 0.0 )
  h1y = ( H[3] , H[4] , H[5] , 0.0 )
  h1z = ( H[6] , H[7] , H[8] , 0.0 )
 }
*/
int2 pixel_position2( const int2 p,
                      const float4 h1x,
                      const float4 h2x,
                      const float4 h3x )
{
  const float4 x = ( float4 )( p.x, p.y, 1.0f, 0.0f ) ;

  const float qx = dot( x, h1x ) ;
  const float qy = dot( x, h2x ) ;
  const float qz = dot( x, h3x ) ;

  if( fabs( qz ) <= 0.00001f )
  {
    return ( int2 )( -1, -1 ) ;
  }
  return ( int2 )( qx / qz, qy / qz ) ;
}

void mat_mul_33( const float * a, const float * b,
                 float * out )
{
  out[0] = a[0] * b[0] + a[1] * b[3] + a[2] * b[6] ;
  out[1] = a[0] * b[1] + a[1] * b[4] + a[2] * b[7] ;
  out[2] = a[0] * b[2] + a[1] * b[5] + a[2] * b[8] ;

  out[3] = a[3] * b[0] + a[4] * b[3] + a[5] * b[6] ;
  out[4] = a[3] * b[1] + a[4] * b[4] + a[5] * b[7] ;
  out[5] = a[3] * b[2] + a[4] * b[5] + a[5] * b[8] ;

  out[6] = a[6] * b[0] + a[7] * b[3] + a[8] * b[6] ;
  out[7] = a[6] * b[1] + a[7] * b[4] + a[8] * b[7] ;
  out[8] = a[6] * b[2] + a[7] * b[5] + a[8] * b[8] ;
}

void mat_mul_vec_33( const float * aMat, const float * aVec, float * aOutVec )
{
  aOutVec[0] = aMat[0] * aVec[0] + aMat[1] * aVec[1] + aMat[2] * aVec[2] ;
  aOutVec[1] = aMat[3] * aVec[0] + aMat[4] * aVec[1] + aMat[5] * aVec[2] ;
  aOutVec[2] = aMat[6] * aVec[0] + aMat[7] * aVec[1] + aMat[8] * aVec[2] ;
}

void mat_scale_in_33( float * out, const float factor )
{
  out[0] = out[0] * factor ;
  out[1] = out[1] * factor ;
  out[2] = out[2] * factor ;

  out[3] = out[3] * factor ;
  out[4] = out[4] * factor ;
  out[5] = out[5] * factor ;

  out[6] = out[6] * factor ;
  out[7] = out[7] * factor ;
  out[8] = out[8] * factor ;
}

void mat_add_33( const float * a, const float * b, float * out )
{
  out[0] = a[0] + b[0] ;
  out[1] = a[1] + b[1] ;
  out[2] = a[2] + b[2] ;

  out[3] = a[3] + b[3] ;
  out[4] = a[4] + b[4] ;
  out[5] = a[5] + b[5] ;

  out[6] = a[6] + b[6] ;
  out[7] = a[7] + b[7] ;
  out[8] = a[8] + b[8] ;
}

void mat_sub_33( const float * a, const float * b, float * out )
{
  out[0] = a[0] - b[0] ;
  out[1] = a[1] - b[1] ;
  out[2] = a[2] - b[2] ;

  out[3] = a[3] - b[3] ;
  out[4] = a[4] - b[4] ;
  out[5] = a[5] - b[5] ;

  out[6] = a[6] - b[6] ;
  out[7] = a[7] - b[7] ;
  out[8] = a[8] - b[8] ;
}

void mat_outer_product_33( const float * a, const float * b, float * out )
{
  out[0] = a[0] * b[0] ;
  out[1] = a[0] * b[1] ;
  out[2] = a[0] * b[2] ;

  out[3] = a[1] * b[0] ;
  out[4] = a[1] * b[1] ;
  out[5] = a[1] * b[2] ;

  out[6] = a[2] * b[0] ;
  out[7] = a[2] * b[1] ;
  out[8] = a[2] * b[2] ;
}

// Compute homography that maps from one image to the other
void compute_homography( const float * R,
                         const float * t,
                         const float * Kref_inv,
                         const float * Kother,
                         const float * plane_n,
                         const float plane_d,
                         float * H )
{
  float tmp[9] ;
  float tmp2[9] ;
  mat_outer_product_33( t, plane_n, tmp ) ;
  mat_scale_in_33( tmp, 1.0f / plane_d ) ;
  mat_sub_33( R, tmp, tmp2 ) ;              // tmp2 = R - t*n / d 
  mat_mul_33( tmp2, Kref_inv, tmp ) ;       // tmp   = ( R - t*n/d ) * Kinv
  mat_mul_33( Kother, tmp, H ) ;            // H     = K * ( R - t*n/d ) * Kinv  
}

// Given a normalized direction, compute x,y such as x,y,n is an orthonormalized frame
void ComputeOrthoNormalizedFrame( const float4 n,
                                  float4 * x,
                                  float4 * y )
{
  if( fabs( n.x ) > fabs( n.y ) )
  {
    *x = ( float4 )( -n.z, 0.f, n.x, 0.f ) / sqrt( n.x * n.x + n.y * n.y ) ;
  }
  else
  {
    *x = ( float4 )( 0.f, n.z, -n.y, 0.f ) / sqrt( n.y * n.y + n.z * n.z ) ;
  }
  *y = cross( n, *x ) ;
}



float4 UniformSampleCone( const float u1, const float u2, const float cos_theta_max )
{
  // @see Total Compendium Dutre, p19

  const float ct  = ( 1.0f - u1 ) + u1 * cos_theta_max ;
  const float st  = sqrt( clamp( 1.0f - ct * ct, 0.0f, 1.0f ) ) ;
  const float phi = u2 * 2.0f * M_PI_F ;

  return ( float4 )( cos( phi ) * st, sin( phi ) * st, ct, 0.f ) ;
}

float4 UniformSampleConeAroundNormal( const float u1, const float u2, const float cos_theta_max, const float4 n )
{
  const float4 dir = UniformSampleCone( u1, u2, cos_theta_max ) ;

  float4 x, y ;
  ComputeOrthoNormalizedFrame( n, &x, &y ) ;

  return ( float4 )( dir.x * x.x + dir.y * y.x + dir.z * n.x,
                     dir.x * x.y + dir.y * y.y + dir.z * n.y,
                     dir.x * x.z + dir.y * y.z + dir.z * n.z,
                     0.f ) ;
}
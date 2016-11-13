#define MAX_COST_PM 10e6 
#define MAX_COST_NCC 2.0f 

#define WINDOW_INCREMENT 2 
#define WINDOW_SIZE 15

/* PROTOTYPES */

/**
* @brief Compute projection from one image to the other using given homography 
* @param p Position in image 1 
* @param H homography 
* @return Position in image 2
* @note If position is invalid (depth is zero), return (-1,-1)
*/
int2 pixel_position( const int2 p ,
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
int2 pixel_position2( const int2 p ,
                      const float4 h1x , 
                      const float4 h2x , 
                      const float4 h3x ) ;

/**
* @brief Compute cost using patch match metric 
* @param Ip Pixel intensity in first image 
* @param Gp Pixel gradient magnitude in first image 
* @param Iq Pixel intensity in second image 
* @param Gq Pixel gradient magnitude in second image 
* @param alpha Balance between gradient and intensity in final cost 
* @param TauImg Threshold on intensity difference 
* @param TauGrad Threshold on gradient difference 
*/
float matching_cost( const float Ip , const float Gp , 
                    const float Iq , const float Gq ,
                    const float alpha , const float TauImg , const float TauGrad ) ;

/**
* @brief Compute cost using patch match metric 
* @param Ip Pixel intensity in first image 
* @param Gpx Pixel gradient magnitude on X in first image 
* @param Gpy Pixel gradient magnitude on Y in the first image 
* @param Iq Pixel intensity in second image 
* @param Gqx Pixel gradient magnitude on X in second image 
* @param Gqy Pixel gradient magnitude on Y in second image 
* @param alpha Balance between gradient and intensity in final cost 
* @param TauImg Threshold on intensity difference 
* @param TauGrad Threshold on gradient difference 
*/
float matching_cost_gx_gy( const float Ip , const float Gpx , const float Gpy , 
                           const float Iq , const float Gqx , const float Gqy ,
                           const float alpha , const float TauImg , const float TauGrad) ;


/**
* @brief Matrix multiplication between two matrices 
* @param a A 3x3 matrix 
* @param b A 3x3 matrix 
* @param[out] A 3x3 matrix, result of the product of a and b 
*/
void mat_mul_33( const float a[9] , const float b[9] ,
                 float out[9] ) ;

/**
* @brief Scale a matrix by a scalar factor 
* @param[in,out] out 3x3 matrix to scale 
* @param factor Scaling factor 
*/
void mat_scale_in_33( float out[9] , const float factor ) ;

/**
* @brief Cwise addition of two matrix 
* @param a A 3x3 matrix 
* @param b A 3x3 matrix 
* @param[out] out Result of a+b 
*/
void mat_add_33( const float a[9] , const float b[9] , float out[9] ) ;

/**
* @brief Cwise subtraction of two matrix 
* @param a A 3x3 matrix 
* @param b A 3x3 matrix 
* @param[out] out Result of a-b 
*/
void mat_sub_33( const float a[9] , const float b[9] , float out[9] ) ;

/**
* @brief Matrix multiplication of two vectors 
* @param a A Column vector 
* @param b a Row vector 
* @param[out] Result of matrix multiplication of a and b 
*/
void mat_outer_product_33( const float a[3] , const float b[3] , float out[9] ) ;

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
void compute_homography( const float * R ,
                         const float * t ,
                         const float * Kref_inv ,
                         const float * Kother ,
                         const float * plane_n ,
                         const float plane_d ,
                         float H[9] ) ;

/**
* @param Sort an array 
* @param[in,out] array The array to sort 
* @param max_nb_elt Number of element in the array 
*/
void sort_array( float * array , const int max_nb_elt ) ;


// NCC functions

/**
* @brief Compute Pixel cost using NCC metric 
* @param pos Position of the pixel to compute 
* @param delta_pos Delta position of the plane to use (relative to the pixel position)
* @param intens_P Intensity of the first image 
* @param intens_Q Intensity of the second image 
* @param planes_n Plane normals 
* @param planes_d Plane parameter 
* @param _R Rotation from first image to the second 
* @param _t Translation from first image to the second 
* @param _Kref_inv Inverse Intrinsic matrix of the first image 
* @param _Kother Inverse Intrinsic matrix of the second image 
* @param[out] outCost Cost array  
*/ 
void compute_pixel_cost_NCC_indiv( const int2 pos , 
                                   const int2 delta_pos , // Delta position to get the plane 
                                      read_only image2d_t intens_P , 
                                      read_only image2d_t intens_Q , 
                                      // The planes 
                                      global float * planes_n , 
                                      global float * planes_d , 
                                      // The stereo rig 
                                      global float * _R ,
                                      global float * _t ,
                                      global float * _Kref_inv ,
                                      global float * _Kother , 
                                      // The output cost image 
                                      global float * outCost ) ;

// General functions 

/**
* @brief Compute plane parameter d using depth, position and normal 
* @param camP Projection matrix (3x4)
* @param camMinv Inverse rotational part of the projection matrix (3x3)
* @param n Plane normal
* @param pos Pixel position 
* @param depth Depth at which intersection is made 
* @note this computes intersection between the ray from position to the plane at specified depth  
*/
float compute_plane_d( const float * camP , 
                       const float * camMinv , 
                       const float4 n , 
                       const int2 pos , 
                       const float depth ) ;

/**
* @brief Convert from depth value to disparity value 
* @param d Depth/disparity 
* @param fx Focal in pixel 
* @param baseline 
* @return Disparity/Depth 
*/
float convert_depth_disparity( const float d , const float fx , const float baseline ) ;

/**
* @brief Get a view vector (from origin to the scene) passing through a pixel 
* @param camP Camera projection matrix (3x4)
* @param camMinv Camera inverse of rotational part (3x3)
* @param camCenter Position of the camera (3x1)
* @param pos Pixel position 
* @return Normalized direction 
*/
float4 compute_view_direction( const float * camP , const float * camMinv , const float4 camCenter , const int2 pos ) ;

/**
* @brief Given a normalized direction, computes two vectors to form an orthonormalized frame 
* @param n Input direction 
* @param x Output x direction 
* @param y Output y direction 
*/
void ComputeOrthoNormalizedFrame( const float4 n , 
                                  float4 * x ,
                                  float4 * y ) ;

/**
* @brief Sample a direction in a cone centered around Z axis 
* @param u1 random value in range [0;1] 
* @param u2 random value in range [0;1] 
* @param cos_theta_max Cosinus of the maximum angle between generated direction and Z axis 
* @return Sampled direction 
*/
float4 UniformSampleCone( const float u1 , const float u2 , const float cos_theta_max ) ;

/**
* @brief Sample a direction in a cone centered around specified direction 
* @param u1 random value in range [0;1]
* @param u2 random value in range [0;1]
* @param cos_theta_max Cosinus of the maximum angle between generated direction and specified direction 
* @param n specified direction 
* @return sampled direction 
*/
float4 UniformSampleConeAroundNormal( const float u1 , const float u2 , const float cos_theta_max , const float4 n ) ;



/*
*
* Implementations 
*
*/

/**
* @brief Compute pixel position given an homography 
* @param p point Position in 2d 
* @param H homography (3x3 matrix)
* @return related point in 2d  
*/
int2 pixel_position( const int2 p ,
                     const float H[9] )
{
  const float x = (float) p.x ;
  const float y = (float) p.y ;

  const float qx = H[0] * x + H[1] * y + H[2] ;
  const float qy = H[3] * x + H[4] * y + H[5] ;
  const float qz = H[6] * x + H[7] * y + H[8] ;

  if( fabs( qz ) <= 0.00001f )
  {
    return (int2) ( -1 , -1 ) ;  
  }
  return (int2)( floor( qx / qz ) , floor( qy / qz ) ) ; 
}

/* Assuming H =
 {
  h1x = ( H[0] , H[1] , H[2] , 0.0 ) 
  h1y = ( H[3] , H[4] , H[5] , 0.0 )
  h1z = ( H[6] , H[7] , H[8] , 0.0 )
 }
*/
int2 pixel_position2( const int2 p ,
                      const float4 h1x , 
                      const float4 h2x , 
                      const float4 h3x ) 
{
  const float4 x = (float4)( p.x , p.y , 1.0f , 0.0f ) ;

  const float qx = dot( x , h1x ) ;
  const float qy = dot( x , h2x ) ;
  const float qz = dot( x , h3x ) ;

  if( fabs( qz ) <= 0.00001f )
  {
    return (int2)( -1 , -1 ) ;
  }
  return (int2)( qx / qz , qy / qz ) ;
}


/**
* @brief Compute (PM) matching cost of a given pair of pixels 
* @param Ip Intensity of first pixel 
* @param Gp Gradient of first pixel 
* @param Iq Intensity of second pixel 
* @param Gq Gradient of second pixel 
* @param alpha Balance between gradient and intensity norm 
* @param TauImg Threshold on intensity 
* @param TauGrad Treshold on Gradient 
*/
float matching_cost( const float Ip , const float Gp , 
                     const float Iq , const float Gq ,
                     const float alpha , const float TauImg , const float TauGrad )
{
  const float diff_intens = (Ip - Iq) ;
  const float diff_grad   = (Gp - Gq) ;

  const float norm_intens = fabs( diff_intens ) ;
  const float norm_grad   = fabs( diff_grad ) ;

  const float cost_intens = fmin( norm_intens , TauImg ) ;
  const float cost_grad   = fmin( norm_grad , TauGrad ) ;

  return ( 1.0f - alpha ) * cost_intens + alpha * cost_grad ;
}

/**
* @brief Compute (PM) matching cost 
* @param Ip 
*/
float matching_cost_gx_gy( const float Ip , const float Gpx , const float Gpy , 
                           const float Iq , const float Gqx , const float Gqy ,
                           const float alpha , const float TauImg , const float TauGrad)
{
  const float Gx = ( Gpx - Gqx ) ;
  const float Gy = ( Gpy - Gqy ) ; 

  const float diff_intens = ( Ip - Iq ) ;
  const float norm_grad   = ( fabs( Gx ) + fabs( Gy ) ) * 0.0625f ;

  const float norm_intens = fabs( diff_intens ) ;

  const float cost_intens = fmin( norm_intens , TauImg ) ;
  const float cost_grad   = fmin( norm_grad , TauGrad ) ;

  return ( ( 1.0f - alpha ) * cost_intens + alpha * cost_grad ) ;
}

void mat_mul_33( const float * a , const float * b ,
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

void mat_mul_vec_33( const float * aMat , const float * aVec , float * aOutVec )
{
  aOutVec[0] = aMat[0] * aVec[0] + aMat[1] * aVec[1] + aMat[2] * aVec[2] ;
  aOutVec[1] = aMat[3] * aVec[0] + aMat[4] * aVec[1] + aMat[5] * aVec[2] ;
  aOutVec[2] = aMat[6] * aVec[0] + aMat[7] * aVec[1] + aMat[8] * aVec[2] ; 
}

void mat_scale_in_33( float * out , const float factor )
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

void mat_add_33( const float * a , const float * b , float * out )
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

void mat_sub_33( const float * a , const float * b , float * out )
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

void mat_outer_product_33( const float * a , const float * b , float * out )
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
void compute_homography( const float * R ,
                         const float * t ,
                         const float * Kref_inv ,
                         const float * Kother ,
                         const float * plane_n ,
                         const float plane_d ,
                         float * H )
{
  float tmp[9] ;
  float tmp2[9] ;
  mat_outer_product_33( t , plane_n , tmp ) ; 
  mat_scale_in_33( tmp , 1.0f / plane_d ) ; 
  mat_sub_33( R , tmp , tmp2 ) ;
  mat_mul_33( tmp2 , Kref_inv , tmp ) ; 
  mat_mul_33( Kother , tmp , H ) ; 
}


void compute_pixel_cost_NCC_indiv( const int2 pos , 
                                   const int2 delta_pos , // Delta position to get the plane 
                                      read_only image2d_t intens_P , 
                                      read_only image2d_t intens_Q , 
                                      // The planes 
                                      global float * planes_n , 
                                      global float * planes_d , 
                                      // The stereo rig 
                                      global float * _R ,
                                      global float * _t ,
                                      global float * _Kref_inv ,
                                      global float * _Kother , 
                                      // The output cost image 
                                      global float * outCost )
{
  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
  const int window_size = WINDOW_SIZE ; 
  const int half_w = window_size / 2 ;
  
  const int p_width = get_image_width( intens_P ) ;
  const int p_height = get_image_height( intens_P ) ;

  const int q_width = get_image_width( intens_Q ) ;
  const int q_height = get_image_height( intens_Q ) ; 

  // Position of the pixel to compute (x,y)
  // Index in the input/output arrays 
  const int pixel_idx = pos.y * p_width + pos.x ; 
  const int2 plane_pos = pos + delta_pos ;
  const int plane_idx = plane_pos.y * p_width + plane_pos.x ; 

  if( plane_pos.x < 0 || plane_pos.y < 0 || plane_pos.x >= p_width || plane_pos.y >= p_height )
  {
    outCost[ pixel_idx ] = MAX_COST_NCC ; 
    return ; 
  }

  // Compute homography for this pixel 
  float H[9] ;
  float plane_n[3] ;
  const float plane_d = planes_d[ plane_idx ] ;
  float R[9] , t[3] , Kref_inv[9] , Kother[9] ;
  // Copy matrices from global to private memory  
  R[0] = _R[0] ; R[1] = _R[1] ; R[2] = _R[2] ;
  R[3] = _R[3] ; R[4] = _R[4] ; R[5] = _R[5] ;
  R[6] = _R[6] ; R[7] = _R[7] ; R[8] = _R[8] ; 
  //
  t[0] = _t[0] ; t[1] = _t[1] ; t[2] = _t[2] ;
  //
  Kref_inv[0] = _Kref_inv[0] ; Kref_inv[1] = _Kref_inv[1] ; Kref_inv[2] = _Kref_inv[2] ;
  Kref_inv[3] = _Kref_inv[3] ; Kref_inv[4] = _Kref_inv[4] ; Kref_inv[5] = _Kref_inv[5] ;
  Kref_inv[6] = _Kref_inv[6] ; Kref_inv[7] = _Kref_inv[7] ; Kref_inv[8] = _Kref_inv[8] ; 
  // 
  Kother[0] = _Kother[0] ; Kother[1] = _Kother[1] ; Kother[2] = _Kother[2] ;
  Kother[3] = _Kother[3] ; Kother[4] = _Kother[4] ; Kother[5] = _Kother[5] ;
  Kother[6] = _Kother[6] ; Kother[7] = _Kother[7] ; Kother[8] = _Kother[8] ; 

  plane_n[0] = planes_n[ 3 * plane_idx ] ;
  plane_n[1] = planes_n[ 3 * plane_idx + 1 ] ;
  plane_n[2] = planes_n[ 3 * plane_idx + 2 ] ; 

  compute_homography( R , t , Kref_inv , Kother , plane_n , plane_d , H ) ;

  // Now compute cost 
  float sum1 = 0.f ; // sum_i v1 
  float sum2 = 0.f ; // sum_i v2 
  float sum3 = 0.f ; // sum_i v1^2 
  float sum4 = 0.f ; // sum_i v1^2 
  float sum5 = 0.f ; // sum_i v1v2 

  int nb = 0 ; 
  for( int y = pos.y - half_w ; y <= pos.y + half_w ; y += WINDOW_INCREMENT )
  {
    for( int x = pos.x - half_w ; x <= pos.x + half_w ; x += WINDOW_INCREMENT )
    {
      const int2 p = (int2)( x , y ); 
      
      if( p.x < 0 || p.x >= p_width || p.y < 0 || p.y >= p_height )
        {
          outCost[ pixel_idx ] = MAX_COST_NCC ; 
          return ; 
        }

      const int2 q = pixel_position( p , H ) ; 

      if( q.x < 0 || q.x >= q_width || q.y < 0 || q.y >= q_height )
      {
        outCost[ pixel_idx ] = MAX_COST_NCC ; 
        return ; 
      }

      const float v1 = read_imagef( intens_P , sampler , p ).r ; 
      const float v2 = read_imagef( intens_Q , sampler , q ).r ; 

      sum1 += v1 ;
      sum2 += v2 ; 
      sum3 += v1 * v1 ;
      sum4 += v2 * v2 ;
      sum5 += v1 * v2 ;

      ++nb ;              
    }
  }

  // Compute NCC cost 
  const float v1m = sum1 / (float) nb ;
  const float v2m = sum2 / (float) nb ; 
  const float v12m = (sum1 * sum1) / (float) nb ;     
  const float v22m = (sum2 * sum2) / (float) nb ; 
  
  const float denom = sqrt( ( sum3 - v12m ) * ( sum4 - v22m ) ) ;
  const float ncc = ( sum5 - ((float) nb) * v1m * v2m ) / denom ;
  if( isinf( ncc ) || isnan( ncc ) )
  {
    outCost[ pixel_idx ] = MAX_COST_NCC ; 
  }
  else 
  {
    outCost[ pixel_idx ] = 1.0f - clamp( ncc , -1.0f , 1.0f ) ; 
  }
}

__kernel void compute_pixel_cost_NCC_red( global int * delta_pos ,
                                          read_only image2d_t intens_P , 
                                          read_only image2d_t intens_Q , 
                                          // The planes 
                                          global float * planes_n , 
                                          global float * planes_d , 
                                          // The stereo rig 
                                          global float * _R ,
                                          global float * _t ,
                                          global float * _Kref_inv ,
                                          global float * _Kother , 
                                          // The output cost image 
                                          global float * outCost )
{
  const int2 pos = (int2)( get_global_id(0) , get_global_id(1) ) ;
  // Handle out of image size 
  const int width = get_image_width( intens_P ) ;
  const int height = get_image_height( intens_P ) ;

  if( pos.x >= width || pos.y >= height || pos.x < 0 || pos.y < 0 )
  {
    return ; 
  }


  /*
     X . X 
     . X . 
     X . X 
  */

  const bool do_computation = ( pos.y % 2 == 0 && pos.x % 2 == 0 ) || 
                              ( pos.y % 2 == 1 && pos.x % 2 == 1 ) ;

  if( do_computation )
  {
    const int2 dp = (int2)( delta_pos[0] , delta_pos[1] ) ; 
    compute_pixel_cost_NCC_indiv( pos , dp , intens_P , intens_Q , planes_n , planes_d , _R , _t , _Kref_inv , _Kother , outCost ) ;
  }
  else 
  {
    outCost[ pos.y * width + pos.x ] = MAX_COST_NCC ;     
  }
}


__kernel void compute_pixel_cost_NCC_black( global int * delta_pos , 
                                            read_only image2d_t intens_P , 
                                            read_only image2d_t intens_Q , 
                                            // The planes 
                                            global float * planes_n , 
                                            global float * planes_d , 
                                            // The stereo rig 
                                            global float * _R ,
                                            global float * _t ,
                                            global float * _Kref_inv ,
                                            global float * _Kother , 
                                            // The output cost image 
                                            global float * outCost )
{
  const int2 pos = (int2)( get_global_id(0) , get_global_id(1) ) ;

  // Handle out of image size 
  const int width = get_image_width( intens_P ) ;
  const int height = get_image_height( intens_P ) ;

  if( pos.x >= width || pos.y >= height || pos.x < 0 || pos.y < 0 )
  {
    return ; 
  }

  /*
     . X . 
     X . X 
     . X . 
  */
  const bool do_computation = ( pos.y % 2 == 0 && pos.x % 2 == 1 ) || 
                              ( pos.y % 2 == 1 && pos.x % 2 == 0 ) ;

  if( do_computation )
  {
    const int2 dp = (int2)( delta_pos[0] , delta_pos[1] ) ; 
    compute_pixel_cost_NCC_indiv( pos , dp , intens_P , intens_Q , planes_n , planes_d , _R , _t , _Kref_inv , _Kother , outCost ) ;
  }
  else 
  {
    outCost[ pos.y * width + pos.x ] = MAX_COST_NCC ;     
  }
}


__kernel void compute_pixel_cost_NCC( read_only image2d_t intens_P , 
                                      read_only image2d_t intens_Q , 
                                      // The planes 
                                      global float * planes_n , 
                                      global float * planes_d , 
                                      // The stereo rig 
                                      global float * _R ,
                                      global float * _t ,
                                      global float * _Kref_inv ,
                                      global float * _Kother , 
                                      // The output cost image 
                                      global float * outCost )
{
  const int2 pos = (int2)( get_global_id(0) , get_global_id(1) ) ;

  // Handle out of image size 
  const int width = get_image_width( intens_P ) ;
  const int height = get_image_height( intens_P ) ;

  // Handle out of image size 
  if( pos.x >= width || pos.y >= height || pos.x < 0 || pos.y < 0 )
  {
    return ; 
  }
  const int2 delta_pos = (int2)( 0 , 0 ) ; 

  compute_pixel_cost_NCC_indiv( pos , delta_pos , intens_P , intens_Q , 
                                planes_n , planes_d , 
                                _R , _t , _Kref_inv , _Kother , outCost ) ;
}



/**
* @brief Kernel that computes cost of a pixel in a stereo rig 
*/
__kernel void compute_pixel_cost_indiv_PM( const int2 pos , 
                                           const int2 delta_pos , 
                                           // Input data images 
                                           read_only image2d_t intens_P , 
                                           read_only image2d_t intens_Q , 
                                           global float * grad_P , 
                                           global float * grad_Q , 
                                           // The planes 
                                           global float * planes_n , 
                                           global float * planes_d , 
                                           // The stereo rig 
                                           global float * _R ,
                                           global float * _t ,
                                           global float * _Kref_inv ,
                                           global float * _Kother , 
                                           // The output cost image 
                                           global float * outCost )
{
  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST; 
  const int window_size = WINDOW_SIZE ; 
  const int half_w = window_size / 2 ; // 11 / 2 
  const float gamma = 10.0f ; 
  
  const int p_width = get_image_width( intens_P ) ;
  const int p_height = get_image_height( intens_P ) ;

  const int q_width = get_image_width( intens_Q ) ;
  const int q_height = get_image_height( intens_Q ) ; 

  // Index in the input/output arrays 
  // Index in the input/output arrays 
  const int pixel_idx = pos.y * p_width + pos.x ; 
  const int2 plane_pos = pos + delta_pos ;
  const int plane_idx = plane_pos.y * p_width + plane_pos.x ; 

  if( plane_pos.x < 0 || plane_pos.y < 0 || plane_pos.x >= p_width || plane_pos.y >= p_height )
  {
    outCost[ pixel_idx ] = MAX_COST_PM ; 
    return ; 
  }


  // PM-Huber value 
  const float alpha = 0.9 ;
  const float TauImg = 10.0f ; 
  const float TauGrad = 2.0f ; 

  // Compute homography for this pixel 
  float H[9] ;
  float plane_n[3] ;
  const float plane_d = planes_d[ plane_idx ] ;
  float R[9] , t[3] , Kref_inv[9] , Kother[9] ;
  // Copy matrices from global to private memory  
  R[0] = _R[0] ; R[1] = _R[1] ; R[2] = _R[2] ;
  R[3] = _R[3] ; R[4] = _R[4] ; R[5] = _R[5] ;
  R[6] = _R[6] ; R[7] = _R[7] ; R[8] = _R[8] ; 
  //
  t[0] = _t[0] ; t[1] = _t[1] ; t[2] = _t[2] ;
  //
  Kref_inv[0] = _Kref_inv[0] ; Kref_inv[1] = _Kref_inv[1] ; Kref_inv[2] = _Kref_inv[2] ;
  Kref_inv[3] = _Kref_inv[3] ; Kref_inv[4] = _Kref_inv[4] ; Kref_inv[5] = _Kref_inv[5] ;
  Kref_inv[6] = _Kref_inv[6] ; Kref_inv[7] = _Kref_inv[7] ; Kref_inv[8] = _Kref_inv[8] ; 
  // 
  Kother[0] = _Kother[0] ; Kother[1] = _Kother[1] ; Kother[2] = _Kother[2] ;
  Kother[3] = _Kother[3] ; Kother[4] = _Kother[4] ; Kother[5] = _Kother[5] ;
  Kother[6] = _Kother[6] ; Kother[7] = _Kother[7] ; Kother[8] = _Kother[8] ; 

  plane_n[0] = planes_n[ 3 * plane_idx ] ;
  plane_n[1] = planes_n[ 3 * plane_idx + 1 ] ;
  plane_n[2] = planes_n[ 3 * plane_idx + 2 ] ; 

  compute_homography( R , t , Kref_inv , Kother , plane_n , plane_d , H ) ;

  // Now compute cost 
  const float Ic = read_imagef( intens_P , sampler , pos ).r * 255.0f ; 

  float res = 0.0f ; 
  for( int y = pos.y - half_w ; y <= pos.y + half_w ; y += WINDOW_INCREMENT )
  {
    for( int x = pos.x - half_w ; x <= pos.x + half_w ; x += WINDOW_INCREMENT )
    {
      const int2 p = { x , y } ;
      
      if( p.x < 0 || p.x >= p_width || p.y < 0 || p.y >= p_height )
        {
          outCost[ pixel_idx ] = MAX_COST_PM ; 
          return ; 
        }

      const int2 q = pixel_position( p , H ) ; 

      if( q.x < 0 || q.x >= q_width || q.y < 0 || q.y >= q_height )
      {
        outCost[ pixel_idx ] = MAX_COST_PM ; 
        return ; 
      }
      
      const int p_idx = y * p_width + x ; 
      const int q_idx = q.y * q_width + q.x ; 

      const float Ip = read_imagef( intens_P , sampler , p ).r * 255.0f ;
      const float Gpx = grad_P[ p_idx * 4 ] ;
      const float Gpy = grad_P[ p_idx * 4 + 1 ] ;
      
      const float Iq = read_imagef( intens_Q , sampler , q ).r * 255.0f ;
      const float Gqx = grad_Q[ q_idx * 4 ] ;
      const float Gqy = grad_Q[ q_idx * 4 + 1 ] ; 

      const float normI = fabs( Ic - Ip ) ;
      const float w = exp( - normI / gamma ) ;
      
      res += w * matching_cost_gx_gy( Ip , Gpx , Gpy , 
                                      Iq , Gqx , Gqy , 
                                      alpha , TauImg , TauGrad ) ;
    }
  } 

  outCost[ pixel_idx ] = res ; 
}

__kernel void compute_pixel_cost_PM( // Input data images 
                                           read_only image2d_t intens_P , 
                                           read_only image2d_t intens_Q , 
                                           global float * grad_P , 
                                           global float * grad_Q , 
                                           // The planes 
                                           global float * planes_n , 
                                           global float * planes_d , 
                                           // The stereo rig 
                                           global float * _R ,
                                           global float * _t ,
                                           global float * _Kref_inv ,
                                           global float * _Kother , 
                                           // The output cost image 
                                           global float * outCost )
{
    const int2 pos = (int2)( get_global_id(0) , get_global_id(1) ) ;

  // Handle out of image size 
  const int width = get_image_width( intens_P ) ;
  const int height = get_image_height( intens_P ) ;

  // Handle out of image size 
  if( pos.x >= width || pos.y >= height || pos.x < 0 || pos.y < 0 )
  {
    return ; 
  }
  const int2 delta_pos = (int2)( 0 , 0 ) ; 


  compute_pixel_cost_indiv_PM( pos , delta_pos , 
                               intens_P , intens_Q , grad_P , grad_Q , 
                               planes_n , planes_d , 
                               _R , _t , 
                               _Kref_inv , _Kother ,
                               outCost ) ; 
}

__kernel void compute_pixel_cost_PM_black( global int * delta_pos , 
                                           read_only image2d_t intens_P , 
                                           read_only image2d_t intens_Q , 
                                           global float * grad_P , 
                                           global float * grad_Q , 
                                           // The planes 
                                           global float * planes_n , 
                                           global float * planes_d , 
                                           // The stereo rig 
                                           global float * _R ,
                                           global float * _t ,
                                           global float * _Kref_inv ,
                                           global float * _Kother , 
                                           // The output cost image 
                                           global float * outCost )
{
  const int2 pos = (int2)( get_global_id(0) , get_global_id(1) ) ;

  // Handle out of image size 
  const int width = get_image_width( intens_P ) ;
  const int height = get_image_height( intens_P ) ;

  // Handle out of image size 
  if( pos.x >= width || pos.y >= height || pos.x < 0 || pos.y < 0 )
  {
    return ; 
  }

    /*
     . X . 
     X . X 
     . X . 
  */
  const bool do_computation = ( pos.y % 2 == 0 && pos.x % 2 == 1 ) || 
                              ( pos.y % 2 == 1 && pos.x % 2 == 0 ) ;

  if( do_computation )
  {
    const int2 dp = (int2)( delta_pos[0] , delta_pos[1] ) ; 
    compute_pixel_cost_indiv_PM( pos , dp , 
                                 intens_P , intens_Q , grad_P , grad_Q , 
                                 planes_n , planes_d , 
                                 _R , _t , _Kref_inv , _Kother , 
                                 outCost ) ; 
  }
  else 
  {
    outCost[ pos.y * width + pos.x ] = MAX_COST_NCC ;     
  }
}

__kernel void compute_pixel_cost_PM_red( global int * delta_pos , 
                                         read_only image2d_t intens_P , 
                                         read_only image2d_t intens_Q , 
                                         global float * grad_P , 
                                         global float * grad_Q , 
                                         // The planes 
                                         global float * planes_n , 
                                         global float * planes_d , 
                                         // The stereo rig 
                                         global float * _R ,
                                         global float * _t ,
                                         global float * _Kref_inv ,
                                         global float * _Kother , 
                                         // The output cost image 
                                         global float * outCost )
{
  const int2 pos = (int2)( get_global_id(0) , get_global_id(1) ) ;

  // Handle out of image size 
  const int width = get_image_width( intens_P ) ;
  const int height = get_image_height( intens_P ) ;

  // Handle out of image size 
  if( pos.x >= width || pos.y >= height || pos.x < 0 || pos.y < 0 )
  {
    return ; 
  }

  /*
     X . X 
     . X . 
     X . X 
  */
  const bool do_computation = ( pos.y % 2 == 0 && pos.x % 2 == 0 ) || 
                              ( pos.y % 2 == 1 && pos.x % 2 == 1 ) ;

  if( do_computation )
  {
    const int2 dp = (int2)( delta_pos[0] , delta_pos[1] ) ; 
    compute_pixel_cost_indiv_PM( pos , dp , 
                                 intens_P , intens_Q , grad_P , grad_Q , 
                                 planes_n , planes_d , 
                                 _R , _t , _Kref_inv , _Kother , 
                                 outCost ) ; 
  }
  else 
  {
    outCost[ pos.y * width + pos.x ] = MAX_COST_NCC ;     
  }
}


/**
* @brief Store cost at a specified positions 
*/
__kernel void store_costs( global float * all_costs ,  // Out 
                           global float * cur_costs , 
                           const int id_cost , const int nb_total_cost , const int width , const int height ,
                           const float max_cost )
{
  const int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ;
  // Index in the input/output arrays 
  const int pixel_idx = pos.y * width + pos.x ; 

  if( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height )
    return ; 

  const float cur_c = cur_costs[ pixel_idx ] ;
  const float val = (isnan( cur_c ) || isinf( cur_c ) || cur_c > max_cost || cur_c < 0.0f) ? max_cost : cur_c ;
  all_costs[ pixel_idx * nb_total_cost + id_cost ] = val ; 
}


// Sort in place an array 
void sort_array( float * array , const int max_nb_elt )
{
  int j ;
  for( int i = 1 ; i < max_nb_elt ; ++i )
  {
    // Try to insert tmp in [0;i] range 
    const float tmp = array[ i ] ; 
    
    // Swap elements until we have elements such as [0;j-1] < tmp <= [j+1;i] 
    for( j = i ; j >= 1 && tmp < array[ j - 1 ] ; --j )
    {
      array[ j ] = array[ j - 1 ] ; 
    }

    // j is where we must insert 
    array[ j ] = tmp ;    
  }
}

__kernel void sort_and_store_costs( global float * final_cost ,
                               global float * all_costs , 
                               const int nb_total_cost_per_pixel , 
                               const int nb_cost_to_use , 
                               const int width , const int height ,
                               const float max_cost )
{
  const int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ;
  // Index in the input/output arrays 
  const int pixel_idx = pos.y * width + pos.x ; 

  if( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height )
    return ; 

  // Retrieve current cost 
  float cur_pixel_costs[ 32 ] ;

  int nb_valid = 0 ; 
  for( int i = 0 ; i < nb_total_cost_per_pixel ; ++i )
  {
    const float cur_c = all_costs[ pixel_idx * nb_total_cost_per_pixel + i ] ;
    nb_valid += ( cur_c < max_cost ) ? 1 : 0 ;
    const float val = cur_c < max_cost ? cur_c : max_cost ; 
    cur_pixel_costs[ i ] = val ;
  }

  sort_array( cur_pixel_costs , nb_total_cost_per_pixel ) ; 

  float sum = 0.0f ; 
  int nb_used = 0 ; 
  for( int i = 0 ; i < nb_cost_to_use && cur_pixel_costs[i] < max_cost && i < nb_valid ; ++i , ++nb_used )
  {
    sum += cur_pixel_costs[ i ] ; 
  }

  sum /= (float) nb_used ;

  const float val = ( nb_used == 0 || sum < 0.f || sum > max_cost ) ? max_cost : sum ; 
  final_cost[ pixel_idx ] = val ; 
}
                               

__kernel void update_plane_wrt_cost( global float * planes_n , global float * planes_d , global int * delta_pos , 
                                     global float * old_cost , global float * new_cost , 
                                     const int width , const int height )
{
  const int2 pos = (int2)( get_global_id(0) , get_global_id(1) ) ;

  if( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height )
  {
    return ; 
  }
  const int pixel_idx = pos.y * width + pos.x ;

  if( new_cost[ pixel_idx ] < old_cost[ pixel_idx ] )
  {
    const int2 dp = (int2)( delta_pos[0] , delta_pos[1] ) ; 
    const int2 pos_plane = pos + dp ;
    if( pos_plane.x < 0 || pos_plane.x >= width || pos_plane.y < 0 || pos_plane.y >= height )
    {
      return ; 
    }  

    old_cost[ pixel_idx ] = new_cost[ pixel_idx ] ;

    const int plane_idx = pos_plane.y * width + pos_plane.x ;

    planes_n[ 3 * pixel_idx ]     = planes_n[ 3 * plane_idx ] ; 
    planes_n[ 3 * pixel_idx + 1 ] = planes_n[ 3 * plane_idx + 1 ] ;
    planes_n[ 3 * pixel_idx + 2 ] = planes_n[ 3 * plane_idx + 2 ] ;

    planes_d[ pixel_idx ] = planes_d[ plane_idx ] ;  
  }
}

__kernel void update_plane_wrt_cost2( global float * cur_planes_n , global float * cur_planes_d , global float * cur_depth ,
                                      global float * test_planes_n , global float * test_planes_d , global float * test_depth ,
                                      global float * old_cost , global float * new_cost , 
                                      const int width , const int height )
{
  const int2 pos = (int2)( get_global_id(0) , get_global_id(1) ) ;

  if( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height )
  {
    return ; 
  }
  const int pixel_idx = pos.y * width + pos.x ;

  if( new_cost[ pixel_idx ] < old_cost[ pixel_idx ] )
  {
    old_cost[ pixel_idx ] = new_cost[ pixel_idx ] ; 

    cur_planes_n[ 3 * pixel_idx ]     = test_planes_n[ 3 * pixel_idx ] ;
    cur_planes_n[ 3 * pixel_idx + 1 ] = test_planes_n[ 3 * pixel_idx + 1 ] ;
    cur_planes_n[ 3 * pixel_idx + 2 ] = test_planes_n[ 3 * pixel_idx + 2 ] ;

    cur_planes_d[ pixel_idx ] = test_planes_d[ pixel_idx ] ;

    cur_depth[ pixel_idx ] = test_depth[ pixel_idx ] ;  
  }  
}

__kernel void compute_pixel_depth( global float * plane_n , global float * plane_d , global float * K , 
                                   const int width , const int height ,
                                   global float * depth )
{
  const int2 pos = (int2)( get_global_id(0) , get_global_id(1) ) ;

  if( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height )
  {
    return ; 
  }

  const int pixel_idx = pos.y * width +  pos.x ; 

  const float fx = K[0] ; 
  const float fy = K[4] ;

  const float u = K[2] ;
  const float v = K[5] ; 

  const float alpha = fx / fy ; 

  const float4 n   = (float4) ( plane_n[ 3 * pixel_idx ] , plane_n[ 3 * pixel_idx + 1 ] , plane_n[ 3 * pixel_idx + 2 ] , 0.0f ) ;
  const float4 pix = (float4) ( pos.x - u , ( pos.y - v ) * alpha , fx , 0.0f ) ;
  depth[ pixel_idx ] = - plane_d[ pixel_idx ] * fx / dot( n , pix ) ;
}

float compute_plane_d( const float * camP , const float * camMinv , const float4 n , const int2 pos , const float depth )
{
  const float pt[3] = { fma( depth , pos.x , -camP[3] ) , 
                        fma( depth , pos.y , -camP[7] ) , 
                        depth         - camP[ 11 ] } ; 

  float _ptX[3] ; 
  mat_mul_vec_33( camMinv , pt , _ptX ) ; 

  const float4 ptX = (float4)( _ptX[0] , _ptX[1] , _ptX[2] , 0.0f ) ; 
  return - ( dot( n , ptX ) ) ; 
}

float convert_depth_disparity( const float d , const float fx , const float baseline )
{
  return fx * baseline / d ; 
}

float4 compute_view_direction( const float * camP , const float * camMinv , const float4 camCenter , const int2 pos )
{
  const float pt[3] = { pos.x - camP[3] , 
                        pos.y - camP[ 7 ] ,
                        1.0f - camP[11] } ;
  float _ptX[3] ;
  mat_mul_vec_33( camMinv , pt , _ptX ) ; 

  const float4 ptX = (float4)( _ptX[0] , _ptX[1] , _ptX[2] , 0.0f ) ; 

  return normalize( ptX - camCenter ) ;
}

// Given a normalized direction, compute x,y such as x,y,n is an orthonormalized frame 
void ComputeOrthoNormalizedFrame( const float4 n , 
                                  float4 * x ,
                                  float4 * y )
{
  if( fabs( n.x ) > fabs( n.y ) )
  {
    *x = (float4)( -n.z , 0.f , n.x , 0.f ) / sqrt( n.x * n.x + n.y * n.y ) ;
  }
  else 
  {
    *x = (float4)( 0.f , n.z , -n.y , 0.f ) / sqrt( n.y * n.y + n.z * n.z ) ; 
  } 
  *y = cross( n , *x ) ; 
}

float4 UniformSampleCone( const float u1 , const float u2 , const float cos_theta_max )
{
    // @see Total Compendium Dutre, p19

    const float ct  = (1.0f - u1) + u1 * cos_theta_max ; 
    const float st  = sqrt( clamp( 1.0f - ct * ct , 0.0f , 1.0f ) ) ; 
    const float phi = u2 * 2.0f * M_PI_F ; 

    return (float4)( cos( phi ) * st , sin( phi ) * st , ct , 0.f ) ; 
}

float4 UniformSampleConeAroundNormal( const float u1 , const float u2 , const float cos_theta_max , const float4 n )
{
  const float4 dir = UniformSampleCone( u1 , u2 , cos_theta_max ) ; 

  float4 x , y ;
  ComputeOrthoNormalizedFrame( n , &x , &y ) ; 

  return (float4)( dir.x * x.x + dir.y * y.x + dir.z * n.x , 
                   dir.x * x.y + dir.y * y.y + dir.z * n.y ,
                   dir.x * x.z + dir.y * y.z + dir.z * n.z ,
                   0.f ) ; 
}

__kernel void compute_new_plane( global float * rnds , global float * cur_depth , global float * cur_normal , 
                                 const float min_disparity , const float max_disparity , 
                                 const float disparity_sampling_range , const float cos_theta_max /* normal_sampling_range */ , 
                                 global float * _camP , global float * _camMinv , global float * _camCenter , 
                                 const float fx , const float baseline , 
                                 const int width , const int height , 
                                 global float * out_plane_n , global float * out_plane_d , global float * out_depth )
{
  const int2 pos = (int2) ( get_global_id( 0 ) , get_global_id( 1 ) ) ;

  if( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height )
  {
    return ; 
  }
  const int pixel_idx = pos.y * width + pos.x ; 

  const int pixel_idx_3 = 3 * pixel_idx ;
  const int pixel_idx_4 = 4 * pixel_idx ; 

  // Retreive the random numbers 
  const float u1 = rnds[ pixel_idx_4 ] ;
  const float u2 = rnds[ pixel_idx_4 + 1 ] ;
  const float u3 = rnds[ pixel_idx_4 + 2 ] ;
  const float u4 = rnds[ pixel_idx_4 + 3 ] ;
  // The camera projection matrix 
  const float camP[12] = { _camP[0] , _camP[1] , _camP[2] , _camP[3] , 
                           _camP[4] , _camP[5] , _camP[6] , _camP[7] ,
                           _camP[8] , _camP[9] , _camP[10] , _camP[11] } ;
  const float camMinv[12] = { _camMinv[0] , _camMinv[1] , _camMinv[2] ,
                              _camMinv[3] , _camMinv[4] , _camMinv[5] ,
                              _camMinv[6] , _camMinv[7] , _camMinv[8] } ; 
  const float4 camPos = (float4)( _camCenter[0] , _camCenter[1] , _camCenter[2] , 0.0f ) ; 

  const float4 cur_n = (float4) ( cur_normal[ pixel_idx_3 ] , 
                                  cur_normal[ pixel_idx_3 + 1 ] , 
                                  cur_normal[ pixel_idx_3 + 2 ] , 
                                  0.0f ) ;
  const float cur_d = cur_depth[ pixel_idx ] ;  

  // 1 - Sample new depth value 
  // Convert depth to disparity 
  const float cur_disp = convert_depth_disparity( cur_d , fx , baseline ) ; 

  // Compute new_disparity 
  const float delta_disp = fma( 2.0f , u1 , -1.0f ) * disparity_sampling_range ; 
  const float new_disparity = clamp( cur_disp + delta_disp , min_disparity , max_disparity ) ; 

  // Convert back to depth value 
  const float new_depth = convert_depth_disparity( new_disparity , fx , baseline ) ; 

  // 2 - Compute new normal 
  /*
  const float dx = fma( 2.0f , u2 , -1.0f ) * normal_sampling_range ; 
  const float dy = fma( 2.0f , u3 , -1.0f ) * normal_sampling_range ;
  const float dz = fma( 2.0f , u4 , -1.0f ) * normal_sampling_range ; 
  */

  const float4 dir   = compute_view_direction( camP , camMinv , camPos , pos ) ;
  float4 new_n = UniformSampleConeAroundNormal( u1 , u2 , cos_theta_max , -dir ) ;  
  // float4 new_n = normalize( cur_n + (float4)( dx , dy , dz , 0.0f ) ) ;

  /*
  if( dot( new_n , dir ) > 0.f )
  {
    new_n = - new_n ; 
  }
  */
  new_n *= ( dot( new_n , dir) > 0.f ) ? -1.0f : 1.0f ;  


  // 3 - compute new plane d 
  // float compute_plane_d( const float * camP , const float * camMinv , const float4 n , const int2 pos , const float depth )
  const float new_plane_d = compute_plane_d( camP , camMinv , new_n , pos , new_depth ) ; 

  // 4 - store back results 
  out_plane_n[ pixel_idx_3 ]     = new_n.x ;
  out_plane_n[ pixel_idx_3 + 1 ] = new_n.y ;
  out_plane_n[ pixel_idx_3 + 2 ] = new_n.z ;

  out_plane_d[ pixel_idx ] = new_plane_d ;

  out_depth[ pixel_idx ] = new_depth ;
}

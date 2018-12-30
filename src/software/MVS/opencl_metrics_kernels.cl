
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
float matching_cost( const float Ip, const float Gp,
                     const float Iq, const float Gq,
                     const float alpha, const float TauImg, const float TauGrad ) ;

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
float matching_cost_gx_gy( const float Ip, const float Gpx, const float Gpy,
                           const float Iq, const float Gqx, const float Gqy,
                           const float alpha, const float TauImg, const float TauGrad ) ;

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
void compute_pixel_cost_NCC_indiv( const int2 pos,
                                   const int2 delta_pos,  // Delta position to get the plane
                                   read_only image2d_t intens_P,
                                   read_only image2d_t intens_Q,
                                   // The planes
                                   const global float * planes_n,
                                   const global float * planes_d,
                                   // The stereo rig
                                   const global float * _R,
                                   const global float * _t,
                                   const global float * _Kref_inv,
                                   const global float * _Kother,
                                   // The output cost image
                                   global float * outCost ) ;




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
float matching_cost( const float Ip, const float Gp,
                     const float Iq, const float Gq,
                     const float alpha, const float TauImg, const float TauGrad )
{
  const float diff_intens = ( Ip - Iq ) ;
  const float diff_grad   = ( Gp - Gq ) ;

  const float norm_intens = fabs( diff_intens ) ;
  const float norm_grad   = fabs( diff_grad ) ;

  const float cost_intens = fmin( norm_intens, TauImg ) ;
  const float cost_grad   = fmin( norm_grad, TauGrad ) ;

  return ( 1.0f - alpha ) * cost_intens + alpha * cost_grad ;
}

/**
* @brief Compute (PM) matching cost
* @param Ip
*/
float matching_cost_gx_gy( const float Ip, const float Gpx, const float Gpy,
                           const float Iq, const float Gqx, const float Gqy,
                           const float alpha, const float TauImg, const float TauGrad )
{
  const float Gx = ( Gpx - Gqx ) ;
  const float Gy = ( Gpy - Gqy ) ;

  const float diff_intens = ( Ip - Iq ) ;
  const float norm_grad   = ( fabs( Gx ) + fabs( Gy ) ) * 0.0625f ;

  const float norm_intens = fabs( diff_intens ) ;

  const float cost_intens = fmin( norm_intens, TauImg ) ;
  const float cost_grad   = fmin( norm_grad, TauGrad ) ;

  return ( ( 1.0f - alpha ) * cost_intens + alpha * cost_grad ) ;
}

// Compute value to a probabilistic value
float proba( const float value, const float lambda )
{
  return 1.0f - exp( - value / lambda ) ;
}


// Census
void compute_pixel_cost_Census_indiv( const int2 pos,
                                      const int2 delta_pos,  // Delta position to get the plane
                                      read_only image2d_t intens_P,
                                      read_only image2d_t intens_Q,
                                      const global unsigned long * census_P,
                                      const global unsigned long * census_Q,
                                      // The planes
                                      const global float * planes_n,
                                      const global float * planes_d,
                                      // The stereo rig
                                      const global float * _R,
                                      const global float * _t,
                                      const global float * _Kref_inv,
                                      const global float * _Kother,
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
    outCost[ pixel_idx ] = MAX_COST_CENSUS ;
    return ;
  }

  // Compute homography for this pixel
  float H[9] ;
  float plane_n[3] ;
  const float plane_d = planes_d[ plane_idx ] ;
  float R[9], t[3], Kref_inv[9], Kother[9] ;
  // Copy matrices from global to private memory
  R[0] = _R[0] ;
  R[1] = _R[1] ;
  R[2] = _R[2] ;
  R[3] = _R[3] ;
  R[4] = _R[4] ;
  R[5] = _R[5] ;
  R[6] = _R[6] ;
  R[7] = _R[7] ;
  R[8] = _R[8] ;
  //
  t[0] = _t[0] ;
  t[1] = _t[1] ;
  t[2] = _t[2] ;
  //
  Kref_inv[0] = _Kref_inv[0] ;
  Kref_inv[1] = _Kref_inv[1] ;
  Kref_inv[2] = _Kref_inv[2] ;
  Kref_inv[3] = _Kref_inv[3] ;
  Kref_inv[4] = _Kref_inv[4] ;
  Kref_inv[5] = _Kref_inv[5] ;
  Kref_inv[6] = _Kref_inv[6] ;
  Kref_inv[7] = _Kref_inv[7] ;
  Kref_inv[8] = _Kref_inv[8] ;
  //
  Kother[0] = _Kother[0] ;
  Kother[1] = _Kother[1] ;
  Kother[2] = _Kother[2] ;
  Kother[3] = _Kother[3] ;
  Kother[4] = _Kother[4] ;
  Kother[5] = _Kother[5] ;
  Kother[6] = _Kother[6] ;
  Kother[7] = _Kother[7] ;
  Kother[8] = _Kother[8] ;

  plane_n[0] = planes_n[ 3 * plane_idx ] ;
  plane_n[1] = planes_n[ 3 * plane_idx + 1 ] ;
  plane_n[2] = planes_n[ 3 * plane_idx + 2 ] ;

  compute_homography( R, t, Kref_inv, Kother, plane_n, plane_d, H ) ;


  // Constants for computation
  const float lambda_census = 30.0f ;
  const float lambda_ad     = 10.0f ;
  const float tau = 60.0f ;

  float total_cost   = 0.0 ;

  for( int y = pos.y - half_w ; y <= pos.y + half_w ; y += WINDOW_INCREMENT )
  {
    for( int x = pos.x - half_w ; x <= pos.x + half_w ; x += WINDOW_INCREMENT )
    {
      const int2 p = ( int2 )( x, y );

      if( p.x < 0 || p.x >= p_width || p.y < 0 || p.y >= p_height )
      {
        outCost[ pixel_idx ] = MAX_COST_CENSUS ;
        return ;
      }

      const int2 q = pixel_position( p, H ) ;

      if( q.x < 0 || q.x >= q_width || q.y < 0 || q.y >= q_height )
      {
        outCost[ pixel_idx ] = MAX_COST_CENSUS ;
        return ;
      }
      const int q_pixel_idx = q.y * q_width + q.x ;

      const float cur_color_p = read_imagef( intens_P, sampler, p ).x ;
      const float cur_color_q = read_imagef( intens_Q, sampler, q ).x ;

      // AD cost
      const float cur_ad = min( tau, 255.0f * fabs( cur_color_p - cur_color_q ) ) ;
      // Census cost
      const int cur_census = popcount( census_P[ pixel_idx ] ^ census_Q[ q_pixel_idx ] ) ;

      total_cost += ( proba( cur_census, lambda_census ) + proba( cur_ad, lambda_ad ) ) ;
    }
  }

  outCost[ pixel_idx ] = total_cost ;
}


__kernel void compute_pixel_cost_Census_red( global int * delta_pos,
    read_only image2d_t intens_P,
    read_only image2d_t intens_Q,
    const global unsigned long * census_P,
    const global unsigned long * census_Q,
    // The planes
    const global float * planes_n,
    const global float * planes_d,
    // The stereo rig
    const global float * _R,
    const global float * _t,
    const global float * _Kref_inv,
    const global float * _Kother,
    // The output cost image
    global float * outCost )
{
  const int2 pos = ( int2 )( get_global_id( 0 ), get_global_id( 1 ) ) ;
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
    const int2 dp = ( int2 )( delta_pos[0], delta_pos[1] ) ;
    compute_pixel_cost_Census_indiv( pos, dp, intens_P, intens_Q, census_P, census_Q, planes_n, planes_d, _R, _t, _Kref_inv, _Kother, outCost ) ;
  }
  else
  {
    outCost[ pos.y * width + pos.x ] = MAX_COST_CENSUS ;
  }
}


__kernel void compute_pixel_cost_Census_black( global int * delta_pos,
    read_only image2d_t intens_P,
    read_only image2d_t intens_Q,
    const global unsigned long * census_P,
    const global unsigned long * census_Q,
    // The planes
    const global float * planes_n,
    const global float * planes_d,
    // The stereo rig
    const global float * _R,
    const global float * _t,
    const global float * _Kref_inv,
    const global float * _Kother,
    // The output cost image
    global float * outCost )
{
  const int2 pos = ( int2 )( get_global_id( 0 ), get_global_id( 1 ) ) ;

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
    const int2 dp = ( int2 )( delta_pos[0], delta_pos[1] ) ;
    compute_pixel_cost_Census_indiv( pos, dp, intens_P, intens_Q, census_P, census_Q, planes_n, planes_d, _R, _t, _Kref_inv, _Kother, outCost ) ;
  }
  else
  {
    outCost[ pos.y * width + pos.x ] = MAX_COST_CENSUS ;
  }
}


__kernel void compute_pixel_cost_Census( read_only image2d_t intens_P,
    read_only image2d_t intens_Q,
    const global unsigned long * census_P,
    const global unsigned long * census_Q,
    // The planes
    global float * planes_n,
    global float * planes_d,
    // The stereo rig
    global float * _R,
    global float * _t,
    global float * _Kref_inv,
    global float * _Kother,
    // The output cost image
    global float * outCost )
{
  const int2 pos = ( int2 )( get_global_id( 0 ), get_global_id( 1 ) ) ;

  // Handle out of image size
  const int width = get_image_width( intens_P ) ;
  const int height = get_image_height( intens_P ) ;

  // Handle out of image size
  if( pos.x >= width || pos.y >= height || pos.x < 0 || pos.y < 0 )
  {
    return ;
  }
  const int2 delta_pos = ( int2 )( 0, 0 ) ;

  compute_pixel_cost_Census_indiv( pos, delta_pos, intens_P, intens_Q, census_P, census_Q,
                                   planes_n, planes_d,
                                   _R, _t, _Kref_inv, _Kother, outCost ) ;
}



/// NCC
void compute_pixel_cost_NCC_indiv( const int2 pos,
                                   const int2 delta_pos,  // Delta position to get the plane
                                   read_only image2d_t intens_P,
                                   read_only image2d_t intens_Q,
                                   // The planes
                                   const global float * planes_n,
                                   const global float * planes_d,
                                   // The stereo rig
                                   const global float * _R,
                                   const global float * _t,
                                   const global float * _Kref_inv,
                                   const global float * _Kother,
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
  float R[9], t[3], Kref_inv[9], Kother[9] ;
  // Copy matrices from global to private memory
  R[0] = _R[0] ;
  R[1] = _R[1] ;
  R[2] = _R[2] ;
  R[3] = _R[3] ;
  R[4] = _R[4] ;
  R[5] = _R[5] ;
  R[6] = _R[6] ;
  R[7] = _R[7] ;
  R[8] = _R[8] ;
  //
  t[0] = _t[0] ;
  t[1] = _t[1] ;
  t[2] = _t[2] ;
  //
  Kref_inv[0] = _Kref_inv[0] ;
  Kref_inv[1] = _Kref_inv[1] ;
  Kref_inv[2] = _Kref_inv[2] ;
  Kref_inv[3] = _Kref_inv[3] ;
  Kref_inv[4] = _Kref_inv[4] ;
  Kref_inv[5] = _Kref_inv[5] ;
  Kref_inv[6] = _Kref_inv[6] ;
  Kref_inv[7] = _Kref_inv[7] ;
  Kref_inv[8] = _Kref_inv[8] ;
  //
  Kother[0] = _Kother[0] ;
  Kother[1] = _Kother[1] ;
  Kother[2] = _Kother[2] ;
  Kother[3] = _Kother[3] ;
  Kother[4] = _Kother[4] ;
  Kother[5] = _Kother[5] ;
  Kother[6] = _Kother[6] ;
  Kother[7] = _Kother[7] ;
  Kother[8] = _Kother[8] ;

  plane_n[0] = planes_n[ 3 * plane_idx ] ;
  plane_n[1] = planes_n[ 3 * plane_idx + 1 ] ;
  plane_n[2] = planes_n[ 3 * plane_idx + 2 ] ;

  compute_homography( R, t, Kref_inv, Kother, plane_n, plane_d, H ) ;

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
      const int2 p = ( int2 )( x, y );

      if( p.x < 0 || p.x >= p_width || p.y < 0 || p.y >= p_height )
      {
        outCost[ pixel_idx ] = MAX_COST_NCC ;
        return ;
      }

      const int2 q = pixel_position( p, H ) ;

      if( q.x < 0 || q.x >= q_width || q.y < 0 || q.y >= q_height )
      {
        outCost[ pixel_idx ] = MAX_COST_NCC ;
        return ;
      }

      const float v1 = read_imagef( intens_P, sampler, p ).x ;
      const float v2 = read_imagef( intens_Q, sampler, q ).x ;

      sum1 += v1 ;
      sum2 += v2 ;
      sum3 += v1 * v1 ;
      sum4 += v2 * v2 ;
      sum5 += v1 * v2 ;

      ++nb ;
    }
  }

  // Compute NCC cost
  const float v1m = sum1 / ( float ) nb ;
  const float v2m = sum2 / ( float ) nb ;
  const float v12m = ( sum1 * sum1 ) / ( float ) nb ;
  const float v22m = ( sum2 * sum2 ) / ( float ) nb ;

  const float denom = sqrt( ( sum3 - v12m ) * ( sum4 - v22m ) ) ;
  const float ncc = ( sum5 - ( ( float ) nb ) * v1m * v2m ) / denom ;
  if( isinf( ncc ) || isnan( ncc ) )
  {
    outCost[ pixel_idx ] = MAX_COST_NCC ;
  }
  else
  {
    outCost[ pixel_idx ] = 1.0f - clamp( ncc, -1.0f, 1.0f ) ;
  }
}

__kernel void compute_pixel_cost_NCC_red( global int * delta_pos,
    read_only image2d_t intens_P,
    read_only image2d_t intens_Q,
    // The planes
    const global float * planes_n,
    const global float * planes_d,
    // The stereo rig
    const global float * _R,
    const global float * _t,
    const global float * _Kref_inv,
    const global float * _Kother,
    // The output cost image
    global float * outCost )
{
  const int2 pos = ( int2 )( get_global_id( 0 ), get_global_id( 1 ) ) ;
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
    const int2 dp = ( int2 )( delta_pos[0], delta_pos[1] ) ;
    compute_pixel_cost_NCC_indiv( pos, dp, intens_P, intens_Q, planes_n, planes_d, _R, _t, _Kref_inv, _Kother, outCost ) ;
  }
  else
  {
    outCost[ pos.y * width + pos.x ] = MAX_COST_NCC ;
  }
}


__kernel void compute_pixel_cost_NCC_black( global int * delta_pos,
    read_only image2d_t intens_P,
    read_only image2d_t intens_Q,
    // The planes
    const global float * planes_n,
    const global float * planes_d,
    // The stereo rig
    const global float * _R,
    const global float * _t,
    const global float * _Kref_inv,
    const global float * _Kother,
    // The output cost image
    global float * outCost )
{
  const int2 pos = ( int2 )( get_global_id( 0 ), get_global_id( 1 ) ) ;

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
    const int2 dp = ( int2 )( delta_pos[0], delta_pos[1] ) ;
    compute_pixel_cost_NCC_indiv( pos, dp, intens_P, intens_Q, planes_n, planes_d, _R, _t, _Kref_inv, _Kother, outCost ) ;
  }
  else
  {
    outCost[ pos.y * width + pos.x ] = MAX_COST_NCC ;
  }
}


__kernel void compute_pixel_cost_NCC( read_only image2d_t intens_P,
                                      read_only image2d_t intens_Q,
                                      // The planes
                                      global float * planes_n,
                                      global float * planes_d,
                                      // The stereo rig
                                      global float * _R,
                                      global float * _t,
                                      global float * _Kref_inv,
                                      global float * _Kother,
                                      // The output cost image
                                      global float * outCost )
{
  const int2 pos = ( int2 )( get_global_id( 0 ), get_global_id( 1 ) ) ;

  // Handle out of image size
  const int width = get_image_width( intens_P ) ;
  const int height = get_image_height( intens_P ) ;

  // Handle out of image size
  if( pos.x >= width || pos.y >= height || pos.x < 0 || pos.y < 0 )
  {
    return ;
  }
  const int2 delta_pos = ( int2 )( 0, 0 ) ;

  compute_pixel_cost_NCC_indiv( pos, delta_pos, intens_P, intens_Q,
                                planes_n, planes_d,
                                _R, _t, _Kref_inv, _Kother, outCost ) ;
}






/**
* @brief Kernel that computes cost of a pixel in a stereo rig
*/
__kernel void compute_pixel_cost_indiv_PM( const int2 pos,
    const int2 delta_pos,
    // Input data images
    read_only image2d_t intens_P,
    read_only image2d_t intens_Q,
    global float * grad_P,
    global float * grad_Q,
    // The planes
    global float * planes_n,
    global float * planes_d,
    // The stereo rig
    global float * _R,
    global float * _t,
    global float * _Kref_inv,
    global float * _Kother,
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
  float R[9], t[3], Kref_inv[9], Kother[9] ;
  // Copy matrices from global to private memory
  R[0] = _R[0] ;
  R[1] = _R[1] ;
  R[2] = _R[2] ;
  R[3] = _R[3] ;
  R[4] = _R[4] ;
  R[5] = _R[5] ;
  R[6] = _R[6] ;
  R[7] = _R[7] ;
  R[8] = _R[8] ;
  //
  t[0] = _t[0] ;
  t[1] = _t[1] ;
  t[2] = _t[2] ;
  //
  Kref_inv[0] = _Kref_inv[0] ;
  Kref_inv[1] = _Kref_inv[1] ;
  Kref_inv[2] = _Kref_inv[2] ;
  Kref_inv[3] = _Kref_inv[3] ;
  Kref_inv[4] = _Kref_inv[4] ;
  Kref_inv[5] = _Kref_inv[5] ;
  Kref_inv[6] = _Kref_inv[6] ;
  Kref_inv[7] = _Kref_inv[7] ;
  Kref_inv[8] = _Kref_inv[8] ;
  //
  Kother[0] = _Kother[0] ;
  Kother[1] = _Kother[1] ;
  Kother[2] = _Kother[2] ;
  Kother[3] = _Kother[3] ;
  Kother[4] = _Kother[4] ;
  Kother[5] = _Kother[5] ;
  Kother[6] = _Kother[6] ;
  Kother[7] = _Kother[7] ;
  Kother[8] = _Kother[8] ;

  plane_n[0] = planes_n[ 3 * plane_idx ] ;
  plane_n[1] = planes_n[ 3 * plane_idx + 1 ] ;
  plane_n[2] = planes_n[ 3 * plane_idx + 2 ] ;

  compute_homography( R, t, Kref_inv, Kother, plane_n, plane_d, H ) ;

  // Now compute cost
  const float Ic = read_imagef( intens_P, sampler, pos ).x * 255.0f ;

  float res = 0.0f ;
  for( int y = pos.y - half_w ; y <= pos.y + half_w ; y += WINDOW_INCREMENT )
  {
    for( int x = pos.x - half_w ; x <= pos.x + half_w ; x += WINDOW_INCREMENT )
    {
      const int2 p = { x, y } ;

      if( p.x < 0 || p.x >= p_width || p.y < 0 || p.y >= p_height )
      {
        outCost[ pixel_idx ] = MAX_COST_PM ;
        return ;
      }

      const int2 q = pixel_position( p, H ) ;

      if( q.x < 0 || q.x >= q_width || q.y < 0 || q.y >= q_height )
      {
        outCost[ pixel_idx ] = MAX_COST_PM ;
        return ;
      }

      const int p_idx = y * p_width + x ;
      const int q_idx = q.y * q_width + q.x ;

      const float Ip = read_imagef( intens_P, sampler, p ).x * 255.0f ;
      const float Gpx = grad_P[ p_idx * 4 ] ;
      const float Gpy = grad_P[ p_idx * 4 + 1 ] ;

      const float Iq = read_imagef( intens_Q, sampler, q ).x * 255.0f ;
      const float Gqx = grad_Q[ q_idx * 4 ] ;
      const float Gqy = grad_Q[ q_idx * 4 + 1 ] ;

      const float normI = fabs( Ic - Ip ) ;
      const float w = exp( - normI / gamma ) ;

      res += w * matching_cost_gx_gy( Ip, Gpx, Gpy,
                                      Iq, Gqx, Gqy,
                                      alpha, TauImg, TauGrad ) ;
    }
  }

  outCost[ pixel_idx ] = res ;
}

__kernel void compute_pixel_cost_PM( // Input data images
  read_only image2d_t intens_P,
  read_only image2d_t intens_Q,
  global float * grad_P,
  global float * grad_Q,
  // The planes
  global float * planes_n,
  global float * planes_d,
  // The stereo rig
  global float * _R,
  global float * _t,
  global float * _Kref_inv,
  global float * _Kother,
  // The output cost image
  global float * outCost )
{
  const int2 pos = ( int2 )( get_global_id( 0 ), get_global_id( 1 ) ) ;

  // Handle out of image size
  const int width = get_image_width( intens_P ) ;
  const int height = get_image_height( intens_P ) ;

  // Handle out of image size
  if( pos.x >= width || pos.y >= height || pos.x < 0 || pos.y < 0 )
  {
    return ;
  }
  const int2 delta_pos = ( int2 )( 0, 0 ) ;


  compute_pixel_cost_indiv_PM( pos, delta_pos,
                               intens_P, intens_Q, grad_P, grad_Q,
                               planes_n, planes_d,
                               _R, _t,
                               _Kref_inv, _Kother,
                               outCost ) ;
}

__kernel void compute_pixel_cost_PM_black( global int * delta_pos,
    read_only image2d_t intens_P,
    read_only image2d_t intens_Q,
    global float * grad_P,
    global float * grad_Q,
    // The planes
    global float * planes_n,
    global float * planes_d,
    // The stereo rig
    global float * _R,
    global float * _t,
    global float * _Kref_inv,
    global float * _Kother,
    // The output cost image
    global float * outCost )
{
  const int2 pos = ( int2 )( get_global_id( 0 ), get_global_id( 1 ) ) ;

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
    const int2 dp = ( int2 )( delta_pos[0], delta_pos[1] ) ;
    compute_pixel_cost_indiv_PM( pos, dp,
                                 intens_P, intens_Q, grad_P, grad_Q,
                                 planes_n, planes_d,
                                 _R, _t, _Kref_inv, _Kother,
                                 outCost ) ;
  }
  else
  {
    outCost[ pos.y * width + pos.x ] = MAX_COST_NCC ;
  }
}

__kernel void compute_pixel_cost_PM_red( global int * delta_pos,
    read_only image2d_t intens_P,
    read_only image2d_t intens_Q,
    global float * grad_P,
    global float * grad_Q,
    // The planes
    global float * planes_n,
    global float * planes_d,
    // The stereo rig
    global float * _R,
    global float * _t,
    global float * _Kref_inv,
    global float * _Kother,
    // The output cost image
    global float * outCost )
{
  const int2 pos = ( int2 )( get_global_id( 0 ), get_global_id( 1 ) ) ;

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
    const int2 dp = ( int2 )( delta_pos[0], delta_pos[1] ) ;
    compute_pixel_cost_indiv_PM( pos, dp,
                                 intens_P, intens_Q, grad_P, grad_Q,
                                 planes_n, planes_d,
                                 _R, _t, _Kref_inv, _Kother,
                                 outCost ) ;
  }
  else
  {
    outCost[ pos.y * width + pos.x ] = MAX_COST_NCC ;
  }
}


__kernel void compute_pixel_cost_Bilateral_NCC( read_only image2d_t intensP,
    read_only image2d_t intensQ,
    // The planes
    global float * planes_n,
    global float * planes_d,
    // The stereo rig
    global float * _R,
    global float * _t,
    global float * _Kref_inv,
    global float * _Kother,
    // The output cost image
    global float * outCost )
{

}

__kernel void compute_pixel_cost_Bilateral_NCC_red( read_only image2d_t intensP,
    read_only image2d_t intensQ,
    // The planes
    global float * planes_n,
    global float * planes_d,
    // The stereo rig
    global float * _R,
    global float * _t,
    global float * _Kref_inv,
    global float * _Kother,
    // The output cost image
    global float * outCost )
{

}

__kernel void compute_pixel_cost_Bilateral_NCC_black( read_only image2d_t intensP,
    read_only image2d_t intensQ,
    // The planes
    global float * planes_n,
    global float * planes_d,
    // The stereo rig
    global float * _R,
    global float * _t,
    global float * _Kref_inv,
    global float * _Kother,
    // The output cost image
    global float * outCost )
{

}
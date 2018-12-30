/**
* @brief Store cost at a specified positions
*/
__kernel void store_costs( global float * all_costs,   // Out
                           global float * cur_costs,
                           const int id_cost, const int nb_total_cost, const int width, const int height,
                           const float max_cost )
{
  const int2 pos = { get_global_id( 0 ), get_global_id( 1 ) } ;
  // Index in the input/output arrays
  const int pixel_idx = pos.y * width + pos.x ;

  if( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height )
  {
    return ;
  }

  const float cur_c = cur_costs[ pixel_idx ] ;
  const float val = ( isnan( cur_c ) || isinf( cur_c ) || cur_c > max_cost || cur_c < 0.0f ) ? max_cost : cur_c ;
  all_costs[ pixel_idx * nb_total_cost + id_cost ] = val ;
}


// Sort in place an array
void sort_array( float * array, const int max_nb_elt )
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

__kernel void sort_and_store_costs( global float * final_cost,
                                    global float * all_costs,
                                    const int nb_total_cost_per_pixel,
                                    const int nb_cost_to_use,
                                    const int width, const int height,
                                    const float max_cost )
{
  const int2 pos = { get_global_id( 0 ), get_global_id( 1 ) } ;
  // Index in the input/output arrays
  const int pixel_idx = pos.y * width + pos.x ;

  if( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height )
  {
    return ;
  }

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

  sort_array( cur_pixel_costs, nb_total_cost_per_pixel ) ;

  float sum = 0.0f ;
  int nb_used = 0 ;
  for( int i = 0 ; i < nb_cost_to_use && cur_pixel_costs[i] < max_cost && i < nb_valid ; ++i, ++nb_used )
  {
    sum += cur_pixel_costs[ i ] ;
  }

  sum /= ( float ) nb_used ;

  const float val = ( nb_used == 0 || sum < 0.f || sum > max_cost ) ? max_cost : sum ;
  final_cost[ pixel_idx ] = val ;
}


__kernel void update_plane_wrt_cost( global float * planes_n, global float * planes_d, global int * delta_pos,
                                     global float * old_cost, global float * new_cost,
                                     const int width, const int height )
{
  const int2 pos = ( int2 )( get_global_id( 0 ), get_global_id( 1 ) ) ;

  if( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height )
  {
    return ;
  }
  const int pixel_idx = pos.y * width + pos.x ;

  if( new_cost[ pixel_idx ] < old_cost[ pixel_idx ] )
  {
    const int2 dp = ( int2 )( delta_pos[0], delta_pos[1] ) ;
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

__kernel void update_plane_wrt_cost2( global float * cur_planes_n, global float * cur_planes_d, global float * cur_depth,
                                      global float * test_planes_n, global float * test_planes_d, global float * test_depth,
                                      global float * old_cost, global float * new_cost,
                                      const int width, const int height )
{
  const int2 pos = ( int2 )( get_global_id( 0 ), get_global_id( 1 ) ) ;

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

__kernel void compute_pixel_depth( global float * plane_n, global float * plane_d, global float * K,
                                   const int width, const int height,
                                   global float * depth )
{
  const int2 pos = ( int2 )( get_global_id( 0 ), get_global_id( 1 ) ) ;

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

  const float4 n   = ( float4 ) ( plane_n[ 3 * pixel_idx ], plane_n[ 3 * pixel_idx + 1 ], plane_n[ 3 * pixel_idx + 2 ], 0.0f ) ;
  const float4 pix = ( float4 ) ( pos.x - u, ( pos.y - v ) * alpha, fx, 0.0f ) ;
  depth[ pixel_idx ] = - plane_d[ pixel_idx ] * fx / dot( n, pix ) ;
}

float compute_plane_d( const float * camP, const float * camMinv, const float4 n, const int2 pos, const float depth )
{
  const float pt[3] = { fma( depth, pos.x, -camP[3] ),
                        fma( depth, pos.y, -camP[7] ),
                        depth         - camP[ 11 ]
                      } ;

  float _ptX[3] ;
  mat_mul_vec_33( camMinv, pt, _ptX ) ;

  const float4 ptX = ( float4 )( _ptX[0], _ptX[1], _ptX[2], 0.0f ) ;
  return - ( dot( n, ptX ) ) ;
}

float convert_depth_disparity( const float d, const float fx, const float baseline )
{
  return fx * baseline / d ;
}

float4 compute_view_direction( const float * camP, const float * camMinv, const float4 camCenter, const int2 pos )
{
  const float pt[3] = { pos.x - camP[3],
                        pos.y - camP[ 7 ],
                        1.0f - camP[11]
                      } ;
  float _ptX[3] ;
  mat_mul_vec_33( camMinv, pt, _ptX ) ;

  const float4 ptX = ( float4 )( _ptX[0], _ptX[1], _ptX[2], 0.0f ) ;

  return normalize( ptX - camCenter ) ;
}



__kernel void compute_new_plane( global float * rnds, global float * cur_depth, global float * cur_normal,
                                 const float min_disparity, const float max_disparity,
                                 const float disparity_sampling_range, const float cos_theta_max /* normal_sampling_range */,
                                 global float * _camP, global float * _camMinv, global float * _camCenter,
                                 const float fx, const float baseline,
                                 const int width, const int height,
                                 global float * out_plane_n, global float * out_plane_d, global float * out_depth )
{
  const int2 pos = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) ) ;

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
  /*
  const float u4 = rnds[ pixel_idx_4 + 3 ] ;
  */
  // The camera projection matrix
  const float camP[12] = { _camP[0], _camP[1], _camP[2], _camP[3],
                           _camP[4], _camP[5], _camP[6], _camP[7],
                           _camP[8], _camP[9], _camP[10], _camP[11]
                         } ;
  const float camMinv[12] = { _camMinv[0], _camMinv[1], _camMinv[2],
                              _camMinv[3], _camMinv[4], _camMinv[5],
                              _camMinv[6], _camMinv[7], _camMinv[8]
                            } ;
  const float4 camPos = ( float4 )( _camCenter[0], _camCenter[1], _camCenter[2], 0.0f ) ;

  const float4 cur_n = ( float4 ) ( cur_normal[ pixel_idx_3 ],
                                    cur_normal[ pixel_idx_3 + 1 ],
                                    cur_normal[ pixel_idx_3 + 2 ],
                                    0.0f ) ;
  const float cur_d = cur_depth[ pixel_idx ] ;

  // 1 - Sample new depth value
  // Convert depth to disparity
  const float cur_disp = convert_depth_disparity( cur_d, fx, baseline ) ;

  // Compute new_disparity
  const float delta_disp = fma( 2.0f, u1, -1.0f ) * disparity_sampling_range ;
  const float new_disparity = clamp( cur_disp + delta_disp, min_disparity, max_disparity ) ;

  // Convert back to depth value
  const float new_depth = convert_depth_disparity( new_disparity, fx, baseline ) ;

  // 2 - Compute new normal
  /*
  const float dx = fma( 2.0f , u2 , -1.0f ) * normal_sampling_range ;
  const float dy = fma( 2.0f , u3 , -1.0f ) * normal_sampling_range ;
  const float dz = fma( 2.0f , u4 , -1.0f ) * normal_sampling_range ;
  */

  const float4 dir   = compute_view_direction( camP, camMinv, camPos, pos ) ;
  float4 new_n = UniformSampleConeAroundNormal( u2, u3, cos_theta_max, cur_n ) ;
  // float4 new_n = normalize( cur_n + (float4)( dx , dy , dz , 0.0f ) ) ;

  /*
  if( dot( new_n , dir ) > 0.f )
  {
    new_n = - new_n ;
  }
  */
  new_n *= ( dot( new_n, dir ) > 0.f ) ? -1.0f : 1.0f ;


  // 3 - compute new plane d
  // float compute_plane_d( const float * camP , const float * camMinv , const float4 n , const int2 pos , const float depth )
  const float new_plane_d = compute_plane_d( camP, camMinv, new_n, pos, new_depth ) ;

  // 4 - store back results
  out_plane_n[ pixel_idx_3 ]     = new_n.x ;
  out_plane_n[ pixel_idx_3 + 1 ] = new_n.y ;
  out_plane_n[ pixel_idx_3 + 2 ] = new_n.z ;

  out_plane_d[ pixel_idx ] = new_plane_d ;

  out_depth[ pixel_idx ] = new_depth ;
}

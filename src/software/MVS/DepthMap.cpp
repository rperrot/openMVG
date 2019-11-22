#include "DepthMap.hpp"

#include "Generators.hpp"
#include "Util.hpp"

#include "openMVG/image/image_io.hpp"
#include "openMVG/image/pixel_types.hpp"
#include "openMVG/numeric/numeric_io_cereal.hpp"

#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/utility.hpp> // std::pair

#include <fstream>
#include <limits>
#include <random>
#include <utility>

namespace MVS
{
/**
* @brief Load a depth map from a file
* @param path Path of the depth map to load
*/
DepthMap::DepthMap( const std::string& path )
{
  if ( !load( path ) )
  {
    std::cerr << "Could not load the depth map file : " << path << std::endl;
  }
}

/**
* @brief Create an empty depth map
* @param height Height of the image
* @param width Width of the image
* @param depth Depth initialization
* @param nor Normal initialization
*/
DepthMap::DepthMap( const int height, const int width, const double depth, const openMVG::Vec4& pl )
    : m_cost( width, height, true, std::numeric_limits<double>::max() ),
      m_depth( width, height, true, depth ),
      m_plane( width, height, true, pl ),
      m_most_important_view( width, height, true, -1 )
{
}

// /**
// * @brief Copy ctr
// * @param src Source
// */
// DepthMap::DepthMap( const DepthMap & src )
//   : m_cost( src.m_cost ) ,
//     m_depth( src.m_depth ) ,
//     m_plane( src.m_plane )
// {

// }

// /**
// * @brief Move ctr
// * @param src Source
// */
// DepthMap::DepthMap( DepthMap && src )
//   : m_cost( std::move( src.m_cost ) ) ,
//     m_depth( std::move( src.m_depth ) ) ,
//     m_plane( std::move( src.m_plane ) )
// {

// }

// /**
// * @brief Assignment operator
// * @param src Source
// * @return self after assignement
// */
// DepthMap & DepthMap::operator=( const DepthMap & src )
// {
//   if( this != &src )
//   {
//     m_cost = src.m_cost ;
//     m_depth = src.m_depth ;
//     m_plane = src.m_plane ;
//   }
//   return *this ;
// }
// /**
// * @brief Move assignment operator
// * @param src Source
// * @return self after assignement
// */
// DepthMap & DepthMap::operator=( DepthMap && src )
// {
//   if( this != &src )
//   {
//     m_cost = std::move( src.m_cost ) ;
//     m_depth = std::move( src.m_depth ) ;
//     m_plane = std::move( src.m_plane ) ;
//   }
//   return *this ;
// }

/**
* @brief Get matching cost at specified position
* @param id_row Row index
* @param id_col Column index
* @return Matching cost at specified position
*/
double DepthMap::cost( const int id_row, const int id_col ) const
{
  return m_cost.coeffRef( id_row, id_col );
}

/**
* @brief Get matching cost at specified position
* @param pos Requested position
* @return Matching cost at specified position
*/
double DepthMap::cost( const openMVG::Vec2i& pos ) const
{
  return cost( pos[ 0 ], pos[ 1 ] );
}

/**
* @brief Set matching cost at specified position
* @param id_row Row index
* @param id_col Column index
* @param new_cost New matching cost
*/
void DepthMap::cost( const int id_row, const int id_col, const double new_cost )
{
  m_cost.coeffRef( id_row, id_col ) = new_cost;
}

/**
* @brief Set matching cost at specified position
* @param pos Requested position
* @param new_cost New matching cost
*/
void DepthMap::cost( const openMVG::Vec2i& pos, const double new_cost )
{
  cost( pos[ 0 ], pos[ 1 ], new_cost );
}

/**
* @brief Get depth value at specified position
* @param id_row Index of the row
* @param id_col Inex of the column
* @return depth at specified position
*/
double DepthMap::depth( const int id_row, const int id_col ) const
{
  return m_depth.coeffRef( id_row, id_col );
}

/**
* @brief Get depth value at specified position
* @param pos Requested position
* @return depth at specified position
*/
double DepthMap::depth( const openMVG::Vec2i& pos ) const
{
  return depth( pos[ 1 ], pos[ 0 ] );
}

/**
* @brief Set Depth value at specified position
* @param id_row Row index
* @param id_col Column index
* @param new_depth New depth value
*/
void DepthMap::depth( const int id_row, const int id_col, const double new_depth )
{
  m_depth.coeffRef( id_row, id_col ) = new_depth;
}

/**
* @brief Set Depth value at specified position
* @param pos Position index
* @param new_depth New depth value
*/
void DepthMap::depth( const openMVG::Vec2i& pos, const double new_depth )
{
  depth( pos[ 0 ], pos[ 1 ], new_depth );
}

/**
* @brief Apply randomization on normals
*/
void DepthMap::randomizePlanes( const Camera& cam, const double min_depth, const double max_depth, const int scale )
{
  // Initialize RNG
  std::mt19937       rng;
  std::random_device device;
  std::seed_seq      seq{device(), device(), device(), device()};
  rng.seed( seq );

  DepthGenerator  d_gen( min_depth, max_depth );
  NormalGenerator n_gen( 80 );

  for ( int id_row = 0; id_row < m_plane.Height(); ++id_row )
  {
    for ( int id_col = 0; id_col < m_plane.Width(); ++id_col )
    {
      openMVG::Vec3 dir = cam.getViewVector( id_col, id_row, scale ); //  cam.getRay( openMVG::Vec2( id_col, id_row ), scale ).second;

      const double        d = d_gen.random( rng );
      const openMVG::Vec3 n = n_gen.random( -dir, rng );

      // Compute plane_d using the current depth
      const double plane_d = GetPlaneD( cam, id_row, id_col, d, n, scale );

      m_plane( id_row, id_col ) = openMVG::Vec4( n[ 0 ], n[ 1 ], n[ 2 ], plane_d );
      m_depth( id_row, id_col ) = d;
    }
  }
}

/**
* @brief Get normal at specified position
* @param id_row Index of the row
* @param id_col Inex of the column
* @return Normal at specified position
*/
openMVG::Vec4 DepthMap::plane( const int id_row, const int id_col ) const
{
  if ( id_row < 0 || id_col < 0 || id_col >= m_plane.Width() || id_row >= m_plane.Height() )
  {
    std::cerr << "Pos : " << id_row << " , " << id_col << "w: " << m_plane.Width() << " h: " << m_plane.Height() << std::endl;
  }
  return m_plane( id_row, id_col );
}

/**
* @brief Get normal at specified position
* @param pos Requested position
* @return Normal at specified position
*/
openMVG::Vec4 DepthMap::plane( const openMVG::Vec2i& pos ) const
{
  return plane( pos[ 1 ], pos[ 0 ] );
}

/**
* @brief Set normal
* @param id_row Row index
* @param id_col Column index
* @param new_normal The new normal
*/
void DepthMap::plane( const int id_row, const int id_col, const openMVG::Vec4& new_normal )
{
  m_plane( id_row, id_col ) = new_normal;
}

/**
* @brief Set normal
* @param pos Requested position
* @param new_normal The new normal
*/
void DepthMap::plane( const openMVG::Vec2i& pos, const openMVG::Vec4& new_normal )
{
  plane( pos[ 1 ], pos[ 0 ], new_normal );
}

/**
   * @brief Set best view for selected pixel 
   * 
   * @param id_row Row index
   * @param id_col Column index
   * 
   * @param best_view The new best view 
   */
void DepthMap::bestView( const int id_row, const int id_col, const int best_view )
{
  m_most_important_view( id_row, id_col ) = best_view;
}

/**
   * @brief Get best view for selected pixel 
   * 
   * @param id_row Row index
   * @param id_col Column index
   * 
   * @return the best view for the given pixel 
   */
int DepthMap::bestView( const int id_row, const int id_col ) const
{
  return m_most_important_view( id_row, id_col );
}

/**
* @brief Indicate if a point is inside the image
* @param id_row Row index
* @param id_col Col index
* @retval true if the point is inside
* @retval false if the point is outside
*/
bool DepthMap::inside( const int id_row, const int id_col ) const
{
  return m_depth.Contains( id_row, id_col );
}

/**
 * @brief Save depth map to a file
 * @param path Path for the output file
 */
bool DepthMap::save( const std::string& path ) const
{
  std::ofstream file( path, std::ios::binary );
  if ( !file )
  {
    std::cerr << "Could not open file : " << path << std::endl;
    return false;
  }

  cereal::PortableBinaryOutputArchive ar( file );

  try
  {
    ar( m_cost );
    ar( m_depth );
    ar( m_plane );
  }
  catch ( ... )
  {
    std::cerr << "Could not serialize-out elements " << std::endl;
    return false;
  }

  return true;
}

/**
* @brief Load a depth map from a file
* @param path Path for the input file
* @retval true If success
* @retval false If failure
*/
bool DepthMap::load( const std::string& path )
{
  std::ifstream file( path, std::ios::binary );
  if ( !file )
  {
    std::cerr << "Could not open file : " << path << std::endl;
    return false;
  }

  cereal::PortableBinaryInputArchive ar( file );

  try
  {
    ar( m_cost );
    ar( m_depth );
    ar( m_plane );
  }
  catch ( ... )
  {
    std::cerr << "Could not serialize-in elements" << std::endl;
    return false;
  }

  return true;
}

/**
* @brief Get width of the dm
* @return width
*/
int DepthMap::width( void ) const
{
  return m_depth.Width();
}

/**
* @brieg Get height of the dm
* @return heigth
*/
int DepthMap::height( void ) const
{
  return m_depth.Height();
}

const openMVG::image::Image<openMVG::Vec4>& DepthMap::planes( void ) const
{
  return m_plane;
}

const openMVG::image::Image<double>& DepthMap::costs( void ) const
{
  return m_cost;
}

/**
* @brief Export to grayscale
*/
void DepthMap::exportToGrayscale( const std::string& path ) const
{
  // Get min - max depth
  double min_depth = std::numeric_limits<double>::max();
  double max_depth = -std::numeric_limits<double>::max();

  for ( int id_row = 0; id_row < m_depth.Height(); ++id_row )
  {
    for ( int id_col = 0; id_col < m_depth.Width(); ++id_col )
    {
      const double d = m_depth( id_row, id_col );
      if ( d > 0.0 )
      {
        min_depth = std::min( min_depth, d );
        max_depth = std::max( max_depth, d );
      }
    }
  }

  openMVG::image::Image<unsigned char> outImg( m_depth.Width(), m_depth.Height() );
  for ( int id_row = 0; id_row < m_depth.Height(); ++id_row )
  {
    for ( int id_col = 0; id_col < m_depth.Width(); ++id_col )
    {
      const double d = std::max( 0.0, m_depth( id_row, id_col ) );

      // range : [0;1]
      const double corrected = ( d - min_depth ) / ( max_depth - min_depth );
      // range : [0;255]
      const int corrected_i = static_cast<int>( corrected * 255.0 );

      const unsigned char val = corrected_i < 0 ? 0 : ( corrected_i > 255 ? 255 : corrected_i );

      outImg( id_row, id_col ) = val;
    }
  }

  WriteImage( path.c_str(), outImg );
}

void DepthMap::exportCost( const std::string& path ) const
{
  // Compute range
  double c_min = std::numeric_limits<double>::max();
  double c_max = -std::numeric_limits<double>::max();

  for ( int id_row = 0; id_row < m_cost.Height(); ++id_row )
  {
    for ( int id_col = 0; id_col < m_cost.Width(); ++id_col )
    {
      if ( m_depth( id_row, id_col ) > 0.0 )
      {
        const double c = m_cost( id_row, id_col );
        c_min          = std::min( c_min, c );
        c_max          = std::max( c_max, c );
      }
    }
  }

  openMVG::image::Image<unsigned char> outImg( m_cost.Width(), m_cost.Height() );

  for ( int id_row = 0; id_row < m_cost.Height(); ++id_row )
  {
    for ( int id_col = 0; id_col < m_cost.Width(); ++id_col )
    {
      if ( m_depth( id_row, id_col ) > 0.0 )
      {
        const double c  = m_cost( id_row, id_col );
        const double cc = c; // c < MAXIMUM_COST ? c : c_max ;

        const double        corrected   = ( cc - c_min ) / ( c_max - c_min );
        const int           corrected_i = static_cast<int>( corrected * 255.0 );
        const unsigned char val         = corrected_i < 0 ? 0 : ( corrected_i > 255 ? 255 : corrected_i );

        outImg( id_row, id_col ) = val;
      }
      else
      {
        outImg( id_row, id_col ) = 0;
      }
    }
  }

  WriteImage( path.c_str(), outImg );
}

void DepthMap::exportNormal( const std::string& path ) const
{
  openMVG::image::Image<openMVG::image::RGBColor> outImg( m_plane.Width(), m_plane.Height() );
  for ( int id_row = 0; id_row < m_plane.Height(); ++id_row )
  {
    for ( int id_col = 0; id_col < m_plane.Width(); ++id_col )
    {
      if ( m_depth( id_row, id_col ) > 0.0 )
      {
        const openMVG::Vec4& pl = m_plane( id_row, id_col );
        const openMVG::Vec3  n( pl[ 0 ], pl[ 1 ], pl[ 2 ] );

        const double r = n[ 0 ];
        const double g = n[ 1 ];
        const double b = n[ 2 ];

        const double cr = ( r + 1.0 ) / 2.0;
        const double cg = ( g + 1.0 ) / 2.0;
        const double cb = ( b + 1.0 ) / 2.0;

        const int cri = static_cast<int>( cr * 255.0 );
        const int cgi = static_cast<int>( cg * 255.0 );
        const int cbi = static_cast<int>( cb * 255.0 );

        const int val_r = Clamp( cri, 0, 255 );
        const int val_g = Clamp( cgi, 0, 255 );
        const int val_b = Clamp( cbi, 0, 255 );

        outImg( id_row, id_col ) = openMVG::image::RGBColor( val_r, val_g, val_b );
      }
      else
      {
        outImg( id_row, id_col ) = openMVG::image::RGBColor( 0, 0, 0 );
      }
    }
  }
  WriteImage( path.c_str(), outImg );
}

/**
 * @brief Export depth map as a ply point cloud
 * @param path Path of the file to export
 * @param cam The camera used to compute point position
 * @param cost_threshold Threshold to remove some points (point with cost above are discarted)
 */
void DepthMap::exportToPly( const std::string& path, const Camera& cam, const double cost_threshold, const int scale )
{
  std::ofstream file( path );
  if ( !file )
  {
    std::cerr << "Unable to open : " << path << std::endl;
    exit( EXIT_FAILURE );
  }
  int                        nb_valid = 0;
  std::vector<openMVG::Vec3> pts;
  for ( int id_row = 0; id_row < m_cost.Height(); ++id_row )
  {
    for ( int id_col = 0; id_col < m_cost.Width(); ++id_col )
    {
      if ( m_cost( id_row, id_col ) < cost_threshold && m_depth( id_row, id_col ) > 0.0 )
      {
        pts.emplace_back( cam.unProject( id_col, id_row, m_depth( id_row, id_col ), scale ) );
        ++nb_valid;
      }
    }
  }

  /* Header */
  file << "ply" << std::endl
       << "format ascii 1.0" << std::endl
       << "element vertex " << nb_valid << std::endl
       << "property float x" << std::endl
       << "property float y" << std::endl
       << "property float z" << std::endl
       << "end_header" << std::endl;

  // All points
  for ( openMVG::Vec3& pt : pts )
  {
    file << pt[ 0 ] << " " << pt[ 1 ] << " " << pt[ 2 ] << std::endl;
  }
}

/**
* @brief Set ground truth depth for known points
* @param cam Camera in which ground truth are known
*/
void DepthMap::setGroundTruthDepth( const Camera& cam, const DepthMapComputationParameters& params, const int scale )
{
  int div = 1;
  for ( int i = 0; i < scale; ++i )
  {
    div *= 2;
  }

  for ( size_t id_pt = 0; id_pt < cam.m_ground_truth.size(); ++id_pt )
  {
    const std::pair<openMVG::Vec2, openMVG::Vec3>& cur_obs = cam.m_ground_truth[ id_pt ];
    const int                                      x       = cur_obs.first[ 0 ] / static_cast<double>( div );
    const int                                      y       = cur_obs.first[ 1 ] / static_cast<double>( div );
    m_depth( y, x )                                        = openMVG::Depth( cam.m_R, cam.m_t, cur_obs.second );
  }
}

/**
 * @brief Scale up depth map
 * @return A scaled up depth map
 * @note Unknown Depth values are interpolated using neighboring depth values
 * @warning Cost value should be recomputed from scratch because interpolation may be wrong
 */
DepthMap DepthMap::upscale( const int target_height, const int target_width ) const
{
  DepthMap res( target_height, target_width );

  /*
  X1,X2,X3,X4 are the existing values

   X1  B  X2
   A   C  _
   X3  _  X4

   * Three scenarii :

   A -> Value is ( X1 + X3 ) / 2
   B -> Value is ( X1 + X2 ) / 2
   C -> Value is ( X1 + X2 + X3 + X4 ) / 4
  */

  // Interpolate values
  for ( int id_row = 0; id_row < res.m_cost.Height(); ++id_row )
  {
    for ( int id_col = 0; id_col < res.m_cost.Width(); ++id_col )
    {
      if ( ( ( id_row % 2 ) == 0 ) && ( ( id_col % 2 ) == 0 ) )
      {
        const int src_row = Clamp( id_row / 2, 0, m_cost.Height() - 1 );
        const int src_col = Clamp( id_col / 2, 0, m_cost.Width() - 1 );

        // Copy the value
        res.m_cost( id_row, id_col )  = m_cost( src_row, src_col );
        res.m_depth( id_row, id_col ) = m_depth( src_row, src_col );
        res.m_plane( id_row, id_col ) = m_plane( src_row, src_col );
      }
      else if ( ( id_row % 2 ) == 0 )
      {
        // Case B
        // Interpolate with left and right values
        const int src_row   = Clamp( id_row / 2, 0, m_cost.Height() - 1 );
        const int src_col_1 = Clamp( id_col / 2, 0, m_cost.Width() - 1 );
        const int src_col_2 = Clamp( src_col_1 + 1, 0, m_cost.Width() - 1 );

        res.m_cost( id_row, id_col )  = ( m_cost( src_row, src_col_1 ) + m_cost( src_row, src_col_2 ) ) / 2.0;
        res.m_depth( id_row, id_col ) = ( m_depth( src_row, src_col_1 ) + m_depth( src_row, src_col_2 ) ) / 2.0;

        openMVG::Vec4       interpolated_plane  = ( m_plane( src_row, src_col_1 ) + m_plane( src_row, src_col_2 ) ) / 2.0;
        const openMVG::Vec3 interpolated_normal = openMVG::Vec3( interpolated_plane[ 0 ], interpolated_plane[ 1 ], interpolated_plane[ 2 ] ).normalized();
        interpolated_plane[ 0 ]                 = interpolated_normal[ 0 ];
        interpolated_plane[ 1 ]                 = interpolated_normal[ 1 ];
        interpolated_plane[ 2 ]                 = interpolated_normal[ 2 ];
        //        interpolated_plane[ 3 ] /= 2.0;

        res.m_plane( id_row, id_col ) = interpolated_plane;
      }
      else if ( ( id_col % 2 ) == 0 )
      {
        // Case A
        // Interpolate with top and bottom values
        const int src_col   = Clamp( id_col / 2, 0, m_cost.Width() - 1 );
        const int src_row_1 = Clamp( id_row / 2, 0, m_cost.Height() - 1 );
        const int src_row_2 = Clamp( src_row_1 + 1, 0, m_cost.Height() - 1 );

        res.m_cost( id_row, id_col )  = ( m_cost( src_row_1, src_col ) + m_cost( src_row_2, src_col ) ) / 2.0;
        res.m_depth( id_row, id_col ) = ( m_depth( src_row_1, src_col ) + m_depth( src_row_2, src_col ) ) / 2.0;

        openMVG::Vec4       interpolated_plane  = ( m_plane( src_row_1, src_col ) + m_plane( src_row_2, src_col ) ) / 2.0;
        const openMVG::Vec3 interpolated_normal = openMVG::Vec3( interpolated_plane[ 0 ], interpolated_plane[ 1 ], interpolated_plane[ 2 ] ).normalized();
        interpolated_plane[ 0 ]                 = interpolated_normal[ 0 ];
        interpolated_plane[ 1 ]                 = interpolated_normal[ 1 ];
        interpolated_plane[ 2 ]                 = interpolated_normal[ 2 ];
        //        interpolated_plane[ 3 ] /= 2.0;

        res.m_plane( id_row, id_col ) = interpolated_plane;
      }
      else
      {
        // id_row % 2 != 0 && id_col %2 != 0
        // Case C
        // Interpolate with the four values
        const int src_row_1 = Clamp( id_row / 2, 0, m_cost.Height() - 1 );
        const int src_row_2 = Clamp( src_row_1 + 1, 0, m_cost.Height() - 1 );
        const int src_col_1 = Clamp( id_col / 2, 0, m_cost.Width() - 1 );
        const int src_col_2 = Clamp( src_col_1 + 1, 0, m_cost.Width() - 1 );

        res.m_cost( id_row, id_col ) =
            ( m_cost( src_row_1, src_col_1 ) +
              m_cost( src_row_1, src_col_2 ) +
              m_cost( src_row_2, src_col_1 ) +
              m_cost( src_row_2, src_col_2 ) ) /
            4.0;
        res.m_depth( id_row, id_col ) =
            ( m_depth( src_row_1, src_col_1 ) +
              m_depth( src_row_1, src_col_2 ) +
              m_depth( src_row_2, src_col_1 ) +
              m_depth( src_row_2, src_col_2 ) ) /
            4.0;

        openMVG::Vec4 interpolated_plane = ( m_plane( src_row_1, src_col_1 ) +
                                             m_plane( src_row_1, src_col_2 ) +
                                             m_plane( src_row_2, src_col_1 ) +
                                             m_plane( src_row_2, src_col_2 ) ) /
                                           4.0;
        const openMVG::Vec3 interpolated_normal = openMVG::Vec3( interpolated_plane[ 0 ], interpolated_plane[ 1 ], interpolated_plane[ 2 ] ).normalized();
        interpolated_plane[ 0 ]                 = interpolated_normal[ 0 ];
        interpolated_plane[ 1 ]                 = interpolated_normal[ 1 ];
        interpolated_plane[ 2 ]                 = interpolated_normal[ 2 ];
        //        interpolated_plane[ 3 ] /= 4.0;

        res.m_plane( id_row, id_col ) = interpolated_plane;
      }
    }
  }

  return res;
}

double MedianFilter( std::vector<double>& val )
{
  const size_t n = val.size() / 2;
  nth_element( val.begin(), val.begin() + n, val.end() );
  return val[ n ];
}

DepthMap DepthMap::medianFilter( const Camera& cam, const int x_size, const int y_size, const int scale ) const
{
  // TODO: this only works for odd sizes
  DepthMap res = *this;

  std::vector<double> tmp( x_size * y_size );
  std::vector<double> tmp2( x_size * y_size );
  std::vector<double> tmp3( x_size * y_size );
  const int           half_size_x = x_size / 2;
  const int           half_size_y = y_size / 2;

  // Filter cost
  for ( int id_row = half_size_y; id_row < m_cost.Height() - half_size_y; ++id_row )
  {
    for ( int id_col = half_size_x; id_col < m_cost.Width() - half_size_x; ++id_col )
    {

      // Get the value
      size_t index = 0;
      for ( int dy = -half_size_y; dy <= half_size_y; ++dy )
      {
        for ( int dx = -half_size_x; dx <= half_size_x; ++dx )
        {
          tmp[ index ] = m_cost( id_row + dy, id_col + dx );
          ++index;
        }
      }
      res.cost( id_row, id_col, MedianFilter( tmp ) );
    }
  }
  // Filter depth
  for ( int id_row = half_size_y; id_row < m_depth.Height() - half_size_y; ++id_row )
  {
    for ( int id_col = half_size_x; id_col < m_depth.Width() - half_size_x; ++id_col )
    {
      // Get the value
      size_t index = 0;
      for ( int dy = -half_size_y; dy <= half_size_y; ++dy )
      {
        for ( int dx = -half_size_x; dx <= half_size_x; ++dx )
        {
          tmp[ index ] = m_depth( id_row + dy, id_col + dx );
          ++index;
        }
      }
      res.depth( id_row, id_col, MedianFilter( tmp ) );
    }
  }
  // Filter planes
  for ( int id_row = half_size_y; id_row < m_depth.Height() - half_size_y; ++id_row )
  {
    for ( int id_col = half_size_x; id_col < m_depth.Width() - half_size_x; ++id_col )
    {
      // Get the value
      size_t index = 0;
      for ( int dy = -half_size_y; dy <= half_size_y; ++dy )
      {
        for ( int dx = -half_size_x; dx <= half_size_x; ++dx )
        {
          tmp[ index ]  = m_plane( id_row + dy, id_col + dx )[ 0 ];
          tmp2[ index ] = m_plane( id_row + dy, id_col + dx )[ 1 ];
          tmp3[ index ] = m_plane( id_row + dy, id_col + dx )[ 2 ];
          index++;
        }
      }
      const double x = MedianFilter( tmp );
      const double y = MedianFilter( tmp2 );
      const double z = MedianFilter( tmp3 );

      const openMVG::Vec3 n = openMVG::Vec3( x, y, z ).normalized();

      const openMVG::Vec3 X = cam.unProject( id_col, id_row, m_depth( id_row, id_col ), scale );
      const double        d = -X.dot( n );
      res.plane( id_row, id_col, openMVG::Vec4( n[ 0 ], n[ 1 ], n[ 2 ], d ) );
    }
  }

  return res;
}

/**
  * @brief Perform filtering on the depth map to keep only values in the depth range 
  * 
  * @param min_th    Minimum depth 
  * @param max_th    Maximum depth 
  */
void DepthMap::filterDepthRange( const double min_th, const double max_th )
{
  for ( int id_row = 0; id_row < m_depth.Height(); ++id_row )
  {
    for ( int id_col = 0; id_col < m_depth.Width(); ++id_col )
    {
      const double d = m_depth( id_row, id_col );
      if ( d < min_th || d > max_th )
      {
        m_depth( id_row, id_col ) = -1.0;
      }
    }
  }
}

std::vector<DepthMap> LoadNeighborDepthMaps( const Camera& cam, const int scale, const DepthMapComputationParameters& params )
{
  std::vector<DepthMap> res;
  for ( size_t id_neigh = 0; id_neigh < cam.m_view_neighbors.size(); ++id_neigh )
  {
    const int         real_id = cam.m_view_neighbors[ id_neigh ];
    const std::string path    = params.getDepthPath( real_id, scale );

    res.emplace_back( DepthMap( path ) );
  }
  return res;
}

} // namespace MVS
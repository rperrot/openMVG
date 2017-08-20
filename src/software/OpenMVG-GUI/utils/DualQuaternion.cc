#include "DualQuaternion.hh"

#include <cmath>

namespace openMVG_gui
{
// Default ctr : identity transformation
DualQuaternion::DualQuaternion( void )
  : m_qr( 1.0 /* w */ , 0.0 /* x */ , 0.0 /* y */ , 0.0 /* z */ ) ,
    m_qd( 0.0 /* w */ , 0.0 /* x */ , 0.0 /* y */ , 0.0 /* z */ )
{

}

// DQ addition
DualQuaternion operator+( const DualQuaternion & dq1 , const DualQuaternion & dq2 )
{
  return DualQuaternion( Eigen::Quaterniond( dq1.m_qr.coeffs() + dq2.m_qr.coeffs() ) ,
                         Eigen::Quaterniond( dq1.m_qd.coeffs() + dq2.m_qd.coeffs() ) ) ;
}

// DQ subtraction
DualQuaternion operator-( const DualQuaternion & dq1 , const DualQuaternion & dq2 )
{
  return DualQuaternion( Eigen::Quaterniond( dq1.m_qr.coeffs() - dq2.m_qr.coeffs() ) ,
                         Eigen::Quaterniond( dq1.m_qd.coeffs() - dq2.m_qd.coeffs() ) ) ;
}


// Scalar multiplication
DualQuaternion operator*( const DualQuaternion & dq , const double s )
{
  return DualQuaternion( Eigen::Quaterniond( dq.m_qr.coeffs() * s ) ,
                         Eigen::Quaterniond( dq.m_qd.coeffs() * s ) ) ;
}

DualQuaternion operator*( const double s , const DualQuaternion & dq )
{
  return DualQuaternion( Eigen::Quaterniond( dq.m_qr.coeffs() * s ) ,
                         Eigen::Quaterniond( dq.m_qd.coeffs() * s ) ) ;
}

// DQ product
DualQuaternion operator*( const DualQuaternion & dq1 , const DualQuaternion & dq2 )
{
  return DualQuaternion( dq1.m_qr * dq2.m_qr ,
                         Eigen::Quaterniond( ( dq1.m_qr * dq2.m_qd ).coeffs() +
                             ( dq1.m_qd * dq2.m_qr ).coeffs() ) ) ;
}

// Conjugate dual quaternion
DualQuaternion DualQuaternion::conjugate( void ) const
{
  return DualQuaternion( m_qr.conjugate() , m_qd.conjugate() ) ;
}

// Normalize a dual quaternion
DualQuaternion DualQuaternion::normalize( void ) const
{
  // Normalization is :
  // |q| = q / || q ||
  // with q = a + eb
  // ||q|| = || a || + a.b / || a ||
  // that leads to :
  // |q| = a / || a || + e ( b / || a || - a / || a || * ( a.b / (|| a || . || a ||) ))
  // if we assume a and b are orthogonal a.b = 0
  // |q| = a / || a || + b / || a ||
  // here we does not assume anything so we use the general form

  const double len = m_qr.norm() ;
  const double sqLen = m_qr.squaredNorm() ;

  DualQuaternion res = *this ;
  res.m_qr.coeffs() /= len ;
  res.m_qd.coeffs() /= len ;
  // The rest is only needed if quaternions ar not orthogonals
  res.m_qd.coeffs() -= ( res.m_qr.coeffs().dot( res.m_qd.coeffs() ) * sqLen ) * res.m_qr.coeffs() ;

  return res ;
}

Eigen::Quaterniond DualQuaternion::rotationalPart( void ) const
{
  return m_qr ;
}

openMVG::Vec3 DualQuaternion::translationalPart( void ) const
{
  Eigen::Quaterniond t( 2.0 * ( m_qd * m_qr.conjugate() ).coeffs() );
  return openMVG::Vec3( t.x() , t.y() , t.z() );
}

// Convert dual quaternion to a (4x4) transformation matrix
openMVG::Mat4 DualQuaternion::toMatrix( void ) const
{
  // DQ must be normalized
  DualQuaternion dqn = normalize() ;

  openMVG::Mat4 res = openMVG::Mat4::Identity() ;

  // Rotational part
  res.block( 0 , 0 , 3 , 3 ) = dqn.m_qr.toRotationMatrix() ;

  // Translational part
  const openMVG::Vec3 t = dqn.translationalPart() ;

  res( 0 , 3 ) = t[0] ;
  res( 1 , 3 ) = t[1] ;
  res( 2 , 3 ) = t[2] ;

  return res ;
}

// Pure rotation around an axis with a specifed angle (in radian)
DualQuaternion DualQuaternion::rotation( const openMVG::Vec3 & axis , const double aRad )
{
  const openMVG::Vec3 n = axis.normalized() ;

  return DualQuaternion( Eigen::Quaterniond( Eigen::AngleAxisd( aRad , n ) ) ,
                         Eigen::Quaterniond( 0.0 , 0.0 , 0.0 , 0.0 ) ) ;
}

// Pure translation of a given vector
DualQuaternion DualQuaternion::translation( const openMVG::Vec3 & d )
{
  const double tx = d[0] / 2.0 ;
  const double ty = d[1] / 2.0 ;
  const double tz = d[2] / 2.0 ;

  return DualQuaternion( Eigen::Quaterniond( 1.0 , 0.0 , 0.0 , 0.0 ) ,
                         Eigen::Quaterniond( 0.0 , tx , ty , tz ) ) ;
}

// FUll ctr
DualQuaternion::DualQuaternion( const Eigen::Quaterniond & qr , const Eigen::Quaterniond & qd )
  : m_qr( qr ) ,
    m_qd( qd )
{

}

/**
* @brief Apply transformation to a point
* @param pt Input point
* @return transformed point
*/
openMVG::Vec3 DualQuaternion::applyPoint( const openMVG::Vec3 & pt ) const
{
  // Apply rotational part
  const DualQuaternion dq = ( *this )
                            *
                            DualQuaternion( Eigen::Quaterniond( 1.0 , 0.0 , 0.0 , 0.0 ) ,
                                Eigen::Quaterniond( 0.0 , pt[0] , pt[1] , pt[2] ) )
                            *
                            conjugate() ;
  openMVG::Vec3 res( dq.m_qd.x() , dq.m_qd.y() , dq.m_qd.z() );

  // apply translational part
  res += 2.0 * ( m_qr.w() * m_qd.vec() -
                 m_qd.w() * m_qr.vec() +
                 m_qr.vec().cross( m_qd.vec() ) ) ;
  return res ;
}

/**
* @brief Apply transformation to a vector
* @param vec Input vector
* @return transformed vector
*/
openMVG::Vec3 DualQuaternion::applyVector( const openMVG::Vec3 & pt ) const
{
  // Same as point but without translation (which is meaningless)
  // Apply rotational part
  const DualQuaternion dq = ( *this )
                            *
                            DualQuaternion( Eigen::Quaterniond( 1.0 , 0.0 , 0.0 , 0.0 ) ,
                                Eigen::Quaterniond( 0.0 , pt[0] , pt[1] , pt[2] ) )
                            *
                            conjugate() ;
  return openMVG::Vec3( dq.m_qd.x() , dq.m_qd.y() , dq.m_qd.z() );
}


} // namespace openMVG_gui
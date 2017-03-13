#include "Similarity3_Kernel.hpp"

#include "openMVG/geometry/rigid_transformation3D_srt.hpp"


namespace openMVG
{
namespace geometry
{
namespace kernel
{

  /**
   * Computes the 3D similarity transform between two point cloud
   *
   * \param x  A 3xN matrix of column vectors.
   * \param y  A 3xN matrix of column vectors.
   * \param sim The found similarity
   *
   * The estimated 3D similarity should approximately hold the condition y = sim(x).
   */
  void Similarity3Solver::Solve
  (
    const Mat &x,
    const Mat &y,
    std::vector<Similarity3> *sims
  )
  {
    assert(3 == x.rows());
    assert(x.rows() == y.rows());
    assert(x.cols() == y.cols());

    double S;
    Vec3 t;
    Mat3 R;
    if ( FindRTS(x, y, &S, &t, &R) )
    {
      // Emplace back the Similarity3
      sims->emplace_back(geometry::Pose3(R, -R.transpose()* t/S), S);
    }
  }


  // Return the Squared error between a collection of points (stored as column)
  Vec Similarity3ErrorSquaredMetric::ErrorVec
  (
    const Similarity3 &S,
    const Mat3X &x1,
    const Mat3X &x2
  )
  {
    return (x2 - S(x1)).colwise().squaredNorm();
  }

  // Return the Squared error between the point x2 and the transformed point S(x1)
  double Similarity3ErrorSquaredMetric::Error
  (
    const Similarity3 &S,
    const Vec3 &x1,
    const Vec3 &x2
  )
  {
    return (x2 - S(x1)).squaredNorm();
  }


  } // namespace kernel
} // namespace geometry
} // namespace openMVG

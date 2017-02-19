#ifndef OPENMVG_STL_NUMERIC_ARRAY_HPP
#define OPENMVG_STL_NUMERIC_ARRAY_HPP

#include <array>

namespace stl
{
/**
  * @brief Simple helper class used to compute some stats on static numeric array (using std::array) 
  * @tparam T internal type 
  * @note only overloaging of 4 or 8 values are supported for now 
  */
template <typename T, size_t N>
class numeric_array
{
public:
  /**
  * @brief Compute median value 
  * @param itb Iterator on the begining of the slice
  * @param ite Iterator on the end of the slice 
  * @note If slice array is odd return the median as usal 
  * @note If slice array is even return the median as the mean of the two median values 
  * @return median value 
  */
  static T median( const typename std::array<T, N>::const_iterator itb,
                   const typename std::array<T, N>::const_iterator ite );
};

// Specialization for array of 4 values
template <typename T>
class numeric_array<T, 4ul>
{
public:
  /**
  * @brief Compute median value 
  * @param itb Iterator on the begining of the slice
  * @param ite Iterator on the end of the slice 
  * @note If slice array is odd return the median as usal 
  * @note If slice array is even return the median as the mean of the two median values 
  * @return median value 
  */
  static T median( const typename std::array<T, 4>::const_iterator itb,
                   const typename std::array<T, 4>::const_iterator ite );
};

/**
  * @brief Compute median value 
  * @param itb Iterator on the begining of the slice
  * @param ite Iterator on the end of the slice 
  * @note If slice array is odd return the median as usal 
  * @note If slice array is even return the median as the mean of the two median values 
  * @return median value 
  */
template <typename T>
T numeric_array<T, 4ul>::median( const typename std::array<T, 4>::const_iterator itb,
                                 const typename std::array<T, 4>::const_iterator ite )
{
  std::array<T, 4> tmp;
  std::copy( itb, ite, tmp.begin() );

  const auto first = tmp.begin();
  const auto end   = tmp.end();
  const auto m1    = first + ( end - first ) / 2;
  const auto m2    = first + ( end - first ) / 2 - 1;

  std::nth_element( first, m1, end );
  const T med1 = *m1;
  std::nth_element( first, m2, end );
  const T med2 = *m2;

  // avoid overflow (a+b) / 2 == min(a,b) + (max(a,b)-min(a,b))/2

  const T m = std::min( med1, med2 );
  const T M = std::max( med1, med2 );

  const T delta = M - m;

  return m + delta / 2;
}

// Specialization for array of 8 values
template <typename T>
class numeric_array<T, 8ul>
{
public:
  /**
  * @brief Compute median value 
  * @param itb Iterator on the begining of the slice
  * @param ite Iterator on the end of the slice 
  * @note If slice array is odd return the median as usal 
  * @note If slice array is even return the median as the mean of the two median values 
  * @return median value 
  */
  static T median( const typename std::array<T, 8>::const_iterator itb,
                   const typename std::array<T, 8>::const_iterator ite );
};

/**
  * @brief Compute median value 
  * @param itb Iterator on the begining of the slice
  * @param ite Iterator on the end of the slice 
  * @note If slice array is odd return the median as usal 
  * @note If slice array is even return the median as the mean of the two median values 
  * @return median value 
  */
template <typename T>
T numeric_array<T, 8ul>::median( const typename std::array<T, 8>::const_iterator itb,
                                 const typename std::array<T, 8>::const_iterator ite )
{
  std::array<T, 8> tmp;
  std::copy( itb, ite, tmp.begin() );

  const auto first = tmp.begin();
  const auto end   = tmp.end();
  const auto m1    = first + ( end - first ) / 2;
  const auto m2    = first + ( end - first ) / 2 - 1;

  std::nth_element( first, m1, end );
  const T med1 = *m1;
  std::nth_element( first, m2, end );
  const T med2 = *m2;

  // avoid overflow (a+b) / 2 == min(a,b) + (max(a,b)-min(a,b))/2

  const T m = std::min( med1, med2 );
  const T M = std::max( med1, med2 );

  const T delta = M - m;

  return m + delta / 2;
}

} // namespace stl

#endif

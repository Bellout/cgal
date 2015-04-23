// Copyright (c) 2014 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
//
//
// Author(s)     : Clement Jamin
//
//******************************************************************************
// File Description :
//
//******************************************************************************

#ifndef CGAL_TC_TEST_TEST_UTILITIES_H
#define CGAL_TC_TEST_TEST_UTILITIES_H

#include <CGAL/Random.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/point_generators_d.h>
#include <CGAL/function_objects.h>
#include <CGAL/Tangential_complex/Point_cloud.h>

// Actually, this is very slow because the "m_points_ds->insert"
// cleans the tree, which is thus built at each query_ANN call
#ifdef CGAL_TC_USE_SLOW_BUT_ACCURATE_SPARSIFIER
template <typename Kernel, typename Point_container>
class Point_sparsifier
{
public:
  typedef typename Kernel::FT                         FT;
  typedef typename Point_container::value_type        Point;
  typedef typename CGAL::Tangential_complex_::Point_cloud_data_structure<
    Kernel, Point_container>                          Points_ds;
  typedef typename Points_ds::KNS_range               KNS_range;

  // We can't instantiate m_points_ds right now since it requires that
  // points is not empty (which be the case here)
  Point_sparsifier(Point_container &points,
                   FT sparsity = FT(0.05*0.05))
    : m_points(points), m_sparsity(sparsity), m_points_ds(NULL)
  {}

  bool try_to_insert_point(const Point &p)
  {
    if (m_points_ds == NULL)
    {
      m_points.push_back(p);
      m_points_ds = new Points_ds(m_points);
      m_points_ds->insert(0);
      return true;
    }
    else
    {
      KNS_range kns_range = m_points_ds->query_ANN(p, 1, false);
      if (kns_range.begin()->second >= m_sparsity)
      {
        m_points.push_back(p);
        m_points_ds->insert(m_points.size() - 1);
        return true;
      }
    }

    return false;
  }

private:
  Point_container     & m_points;
  Points_ds           * m_points_ds;
  FT                    m_sparsity;
};
#endif

// construct_point: dim 2
template <typename Kernel>
typename Kernel::Point_d construct_point(
  const Kernel &k,
  typename Kernel::FT x1, typename Kernel::FT x2)
{
  typename Kernel::FT tab[2];
  tab[0] = x1; tab[1] = x2;
  return k.construct_point_d_object()(2, &tab[0], &tab[2]);
}

// construct_point: dim 3
template <typename Kernel>
typename Kernel::Point_d construct_point(
  const Kernel &k,
  typename Kernel::FT x1, typename Kernel::FT x2, typename Kernel::FT x3)
{
  typename Kernel::FT tab[3];
  tab[0] = x1; tab[1] = x2; tab[2] = x3;
  return k.construct_point_d_object()(3, &tab[0], &tab[3]);
}

// construct_point: dim 4
template <typename Kernel>
typename Kernel::Point_d construct_point(
  const Kernel &k,
  typename Kernel::FT x1, typename Kernel::FT x2, typename Kernel::FT x3,
  typename Kernel::FT x4)
{
  typename Kernel::FT tab[4];
  tab[0] = x1; tab[1] = x2; tab[2] = x3; tab[3] = x4;
  return k.construct_point_d_object()(4, &tab[0], &tab[4]);
}

// construct_point: dim 5
template <typename Kernel>
typename Kernel::Point_d construct_point(
  const Kernel &k,
  typename Kernel::FT x1, typename Kernel::FT x2, typename Kernel::FT x3,
  typename Kernel::FT x4, typename Kernel::FT x5)
{
  typename Kernel::FT tab[5];
  tab[0] = x1; tab[1] = x2; tab[2] = x3; tab[3] = x4; tab[4] = x5;
  return k.construct_point_d_object()(5, &tab[0], &tab[5]);
}

template <typename Kernel, typename Point_container>
std::vector<typename Point_container::value_type>
sparsify_point_set(
  const Kernel &k, Point_container const& input_pts,
  typename Kernel::FT min_squared_dist)
{
  typedef typename CGAL::Tangential_complex_::Point_cloud_data_structure<
    Kernel, Point_container>                    Points_ds;
  typedef typename Points_ds::INS_iterator      INS_iterator;
  typedef typename Points_ds::INS_range         INS_range;

  typename Kernel::Squared_distance_d sqdist = k.squared_distance_d_object();

#ifdef CGAL_TC_PROFILING
    Wall_clock_timer t;
#endif

  // Create the output container
  std::vector<typename Point_container::value_type> output;

  Points_ds points_ds(input_pts);

  std::vector<bool> dropped_points(input_pts.size(), false);

  // Parse the following points, and add them if they are not too close to
  // the other points
  std::size_t pt_idx = 0;
  for (typename Point_container::const_iterator it_pt = input_pts.begin() ;
       it_pt != input_pts.end();
       ++it_pt, ++pt_idx)
  {
    if (dropped_points[pt_idx])
      continue;

    output.push_back(*it_pt);

    INS_range ins_range = points_ds.query_incremental_ANN(*it_pt);

    // Drop it if there is another point that:
    // - is closer that min_squared_dist
    // - and has a higher index
    for (INS_iterator nn_it = ins_range.begin() ;
        nn_it != ins_range.end() ;
        ++nn_it)
    {
      std::size_t neighbor_point_idx = nn_it->first;
      typename Kernel::FT sq_dist = nn_it->second;
      // The neighbor is too close, we drop the neighbor
      if (sq_dist < min_squared_dist)
        dropped_points[neighbor_point_idx] = true;
      else
        break;
    }
  }

#ifdef CGAL_TC_PROFILING
    std::cerr << "Point set sparsified in " << t.elapsed()
              << " seconds." << std::endl;
#endif

  return output;
}

template<typename Point, typename OutputIterator>
bool load_points_from_file(
  const std::string &filename, OutputIterator points,
  std::size_t only_first_n_points = std::numeric_limits<std::size_t>::max())
{
  std::ifstream in(filename);
  if (!in.is_open())
  {
    std::cerr << "Could not open '" << filename << "'" << std::endl;
    return false;
  }

  Point p;
  int dim_from_file;
  in >> dim_from_file;

  std::size_t i = 0;
  while(i < only_first_n_points && in >> p)
  {
    *points++ = p;
    ++i;
  }

#ifdef CGAL_TC_VERBOSE
  std::cerr << "'" << filename << "' loaded." << std::endl;
#endif

  return true;
}

template <typename Kernel>
std::vector<typename Kernel::Point_d> generate_points_on_plane(std::size_t num_points)
{
  typedef typename Kernel::Point_d Point;
  typedef typename Kernel::FT FT;
  Kernel k;
  CGAL::Random rng;
  std::vector<Point> points;
  points.reserve(num_points);
#ifdef CGAL_TC_USE_SLOW_BUT_ACCURATE_SPARSIFIER
  Point_sparsifier<Kernel, std::vector<Point> > sparsifier(points);
#endif
  for (std::size_t i = 0 ; i < num_points ; )
  {
    FT x = rng.get_double(0, 5);
    FT y = rng.get_double(0, 5);
    Point p = construct_point(k, x, y, FT(0));
#ifdef CGAL_TC_USE_SLOW_BUT_ACCURATE_SPARSIFIER
    if (sparsifier.try_to_insert_point(p))
      ++i;
#else
    points.push_back(p);
    ++i;
#endif
  }
  return points;
}

template <typename Kernel>
std::vector<typename Kernel::Point_d> generate_points_on_moment_curve(
  std::size_t num_points, int dim,
  typename Kernel::FT min_x , typename Kernel::FT max_x)
{
  typedef typename Kernel::Point_d Point;
  typedef typename Kernel::FT FT;
  Kernel k;
  CGAL::Random rng;
  std::vector<Point> points;
  points.reserve(num_points);
#ifdef CGAL_TC_USE_SLOW_BUT_ACCURATE_SPARSIFIER
  Point_sparsifier<Kernel, std::vector<Point> > sparsifier(points);
#endif
  for (std::size_t i = 0 ; i < num_points ; )
  {
    FT x = rng.get_double(min_x, max_x);
    std::vector<FT> coords;
    coords.reserve(dim);
    for (int p = 1 ; p <= dim ; ++p)
      coords.push_back(std::pow(CGAL::to_double(x), p));
    Point p = k.construct_point_d_object()(
      dim, coords.begin(), coords.end());
#ifdef CGAL_TC_USE_SLOW_BUT_ACCURATE_SPARSIFIER
    if (sparsifier.try_to_insert_point(p))
      ++i;
#else
    points.push_back(p);
    ++i;
#endif
  }
  return points;
}

template <typename Kernel>
std::vector<typename Kernel::Point_d> generate_points_on_sphere_d(
  std::size_t num_points, int dim, double radius, 
  double radius_noise_percentage = 0.)
{
  typedef typename Kernel::Point_d Point;
  Kernel k;
  CGAL::Random rng;
  CGAL::Random_points_on_sphere_d<Point> generator(dim, radius);
  std::vector<Point> points;
  points.reserve(num_points);
#ifdef CGAL_TC_USE_SLOW_BUT_ACCURATE_SPARSIFIER
  Point_sparsifier<Kernel, std::vector<Point> > sparsifier(points);
#endif
  for (std::size_t i = 0 ; i < num_points ; )
  {
    Point p = *generator++;
    if (radius_noise_percentage > 0.)
    { 
      double radius_noise_ratio = rng.get_double(
        (100. - radius_noise_percentage)/100., 
        (100. + radius_noise_percentage)/100.);
      
      typename Kernel::Point_to_vector_d k_pt_to_vec =
        k.point_to_vector_d_object();
      typename Kernel::Vector_to_point_d k_vec_to_pt =
        k.vector_to_point_d_object();
      typename Kernel::Scaled_vector_d k_scaled_vec =
        k.scaled_vector_d_object();
      p = k_vec_to_pt(k_scaled_vec(k_pt_to_vec(p), radius_noise_ratio));
    }
#ifdef CGAL_TC_USE_SLOW_BUT_ACCURATE_SPARSIFIER
    if (sparsifier.try_to_insert_point(p))
      ++i;
#else
    points.push_back(p);
    ++i;
#endif
  }
  return points;
}

// a = big radius, b = small radius
template <typename Kernel>
std::vector<typename Kernel::Point_d> generate_points_on_klein_bottle_3D(
  std::size_t num_points, double a, double b, bool uniform = false)
{
  typedef typename Kernel::Point_d Point;
  typedef typename Kernel::FT FT;
  Kernel k;
  CGAL::Random rng;

  // if uniform
  std::size_t num_lines = (std::size_t)sqrt(num_points);
  std::size_t num_cols = num_points/num_lines + 1;

  std::vector<Point> points;
  points.reserve(num_points);
#ifdef CGAL_TC_USE_SLOW_BUT_ACCURATE_SPARSIFIER
  Point_sparsifier<Kernel, std::vector<Point> > sparsifier(points);
#endif
  for (std::size_t i = 0 ; i < num_points ; )
  {
    FT u, v;
    if (uniform)
    {
      std::size_t k1 = i / num_lines;
      std::size_t k2 = i % num_lines;
      u = 6.2832 * k1 / num_lines;
      v = 6.2832 * k2 / num_lines;
    }
    else
    {
      u = rng.get_double(0, 6.2832);
      v = rng.get_double(0, 6.2832);
    }
    double tmp = cos(u/2)*sin(v) - sin(u/2)*sin(2.*v);
    Point p = construct_point(k,
      (a + b*tmp)*cos(u),
      (a + b*tmp)*sin(u),
      b*(sin(u/2)*sin(v) + cos(u/2)*sin(2.*v)));
#ifdef CGAL_TC_USE_SLOW_BUT_ACCURATE_SPARSIFIER
    if (sparsifier.try_to_insert_point(p))
      ++i;
#else
    points.push_back(p);
    ++i;
#endif
  }
  return points;
}

// a = big radius, b = small radius
template <typename Kernel>
std::vector<typename Kernel::Point_d> generate_points_on_klein_bottle_4D(
  std::size_t num_points, double a, double b, double noise = 0., bool uniform = false)
{
  typedef typename Kernel::Point_d Point;
  typedef typename Kernel::FT FT;
  Kernel k;
  CGAL::Random rng;

  // if uniform
  std::size_t num_lines = (std::size_t)sqrt(num_points);
  std::size_t num_cols = num_points/num_lines + 1;

  std::vector<Point> points;
  points.reserve(num_points);
#ifdef CGAL_TC_USE_SLOW_BUT_ACCURATE_SPARSIFIER
  Point_sparsifier<Kernel, std::vector<Point> > sparsifier(points);
#endif
  for (std::size_t i = 0 ; i < num_points ; )
  {
    FT u, v;
    if (uniform)
    {
      std::size_t k1 = i / num_lines;
      std::size_t k2 = i % num_lines;
      u = 6.2832 * k1 / num_lines;
      v = 6.2832 * k2 / num_lines;
    }
    else
    {
      u = rng.get_double(0, 6.2832);
      v = rng.get_double(0, 6.2832);
    }
    Point p = construct_point(k,
        (a + b*cos(v))*cos(u) + (noise == 0. ? 0. : rng.get_double(0, noise)),
        (a + b*cos(v))*sin(u) + (noise == 0. ? 0. : rng.get_double(0, noise)),
        b*sin(v)*cos(u/2)     + (noise == 0. ? 0. : rng.get_double(0, noise)),
        b*sin(v)*sin(u/2)     + (noise == 0. ? 0. : rng.get_double(0, noise)));
#ifdef CGAL_TC_USE_SLOW_BUT_ACCURATE_SPARSIFIER
    if (sparsifier.try_to_insert_point(p))
      ++i;
#else
    points.push_back(p);
    ++i;
#endif
  }
  return points;
}


// a = big radius, b = small radius
template <typename Kernel>
std::vector<typename Kernel::Point_d>
generate_points_on_klein_bottle_variant_5D(
  std::size_t num_points, double a, double b, bool uniform = false)
{
  typedef typename Kernel::Point_d Point;
  typedef typename Kernel::FT FT;
  Kernel k;
  CGAL::Random rng;

  // if uniform
  std::size_t num_lines = (std::size_t)sqrt(num_points);
  std::size_t num_cols = num_points/num_lines + 1;

  std::vector<Point> points;
  points.reserve(num_points);
#ifdef CGAL_TC_USE_SLOW_BUT_ACCURATE_SPARSIFIER
  Point_sparsifier<Kernel, std::vector<Point> > sparsifier(points);
#endif
  for (std::size_t i = 0 ; i < num_points ; )
  {
    FT u, v;
    if (uniform)
    {
      std::size_t k1 = i / num_lines;
      std::size_t k2 = i % num_lines;
      u = 6.2832 * k1 / num_lines;
      v = 6.2832 * k2 / num_lines;
    }
    else
    {
      u = rng.get_double(0, 6.2832);
      v = rng.get_double(0, 6.2832);
    }
    FT x1 = (a + b*cos(v))*cos(u);
    FT x2 = (a + b*cos(v))*sin(u);
    FT x3 = b*sin(v)*cos(u/2);
    FT x4 = b*sin(v)*sin(u/2);
    FT x5 = x1 + x2 + x3 + x4;

    Point p = construct_point(k, x1, x2, x3, x4, x5);
#ifdef CGAL_TC_USE_SLOW_BUT_ACCURATE_SPARSIFIER
    if (sparsifier.try_to_insert_point(p))
      ++i;
#else
    points.push_back(p);
    ++i;
#endif
  }
  return points;
}

#endif // CGAL_MESH_3_TEST_TEST_UTILITIES_H

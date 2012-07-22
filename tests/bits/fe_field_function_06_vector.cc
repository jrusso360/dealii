//-------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2005, 2011, 2012 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//-------------------------------------------------------


// FEFieldFunction ran into an assertion after
// Mapping::transform_real_to_unit_cell started throwing exceptions
// when it couldn't find the point on the reference cell that belongs
// to a given point, rather than just silently giving up
//
// this is yet another variant of _04 and _05 found by inspecting the
// code in FEFieldFunction but the (same) exception is triggered in a
// different place

#include "../tests.h"

#include <deal.II/base/utilities.h>
#include <deal.II/grid/tria.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/fe/mapping_q1.h>
#include <deal.II/grid/tria_boundary_lib.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/numerics/fe_field_function.h>
#include <deal.II/numerics/vectors.h>
#include <deal.II/fe/fe_system.h>


template <int dim>
class F : public Function<dim>
{
  public:
    F() : Function<dim>(2) {}
    virtual void vector_value (const Point<dim> &p,
			       Vector<double> &v) const
      {
	v = 0;
	v[0] = p.square();
      }
};



template<int dim>
void test()
{
  const HyperBallBoundary<dim> boundary_description;

  Triangulation<dim>   triangulation;
  GridGenerator::hyper_ball (triangulation);
  triangulation.set_boundary (0, boundary_description);
  triangulation.refine_global (1);

  FESystem<dim> fe(FE_Q<dim> (2),2);
  DoFHandler<dim> dof_handler(triangulation);
  dof_handler.distribute_dofs(fe);

				   // interpolate a quadratic function
				   // into the space; this function
				   // can be represented exactly, so
				   // that we can compare again later
  Vector<double> solution(dof_handler.n_dofs());
  VectorTools::interpolate (dof_handler, F<dim>(), solution);

  Functions::FEFieldFunction<2> fe_function (dof_handler, solution);

				   // add only one points but also set
				   // the active cell to one that
				   // doesn't contain the current
				   // point.  the problem happens
				   // because we walk over a bunch of
				   // cells in the process of finding
				   // all of these points and then
				   // realize when we get to the one
				   // at the end that the coordinates
				   // for this point can't be found in
				   // the cell we have touched last
				   // (it's too far away from that
				   // cell, and the inverse mapping
				   // does not converge
  Point<dim> point(-0.27999999999999992, -0.62999999999999989);
  fe_function.set_active_cell (typename DoFHandler<dim>::active_cell_iterator
			       (&triangulation,
				1,
				4,
				&dof_handler));

  Vector<double> m(2);
  fe_function.vector_value (point, m);

  {
    Assert (std::fabs(m(0) - point.square())
	    <
	    1e-10 * std::fabs(m(0) + point.square()),
	    ExcInternalError());

    Assert (std::fabs(m(1))
	    <
	    1e-10,
	    ExcInternalError());
  }

  deallog << "OK" << std::endl;
}


int main ()
{
  std::ofstream logfile("fe_field_function_06_vector/output");
  deallog.attach(logfile);
  deallog.depth_console(0);

  test<2>();

  return 0;
}

//----------------------------  find_cell_5.cc  ---------------------------
//    $Id: find_cell_5.cc 25229 2012-03-09 18:34:59Z pauletti $
//    Version: $Name$ 
//
//    Copyright (C) 2003, 2004, 2005, 2006, 2012 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  find_cell_5.cc  ---------------------------

// find_active_cell_around_point() stops working if some cells are refined in 3d.
// this is caused by a bug in GridTools::find_cells_adjacent_to_vertex that got fixed in r 25562.

#include "../tests.h"
#include <deal.II/base/logstream.h>
#include <deal.II/grid/tria.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_in.h>
#include <deal.II/grid/tria_boundary_lib.h>

#include <fstream>

#include <deal.II/fe/mapping_q1.h>

bool inside(Triangulation<3> &tria, Point<3> &p)
{
  
  for(Triangulation<3>::cell_iterator cell = tria.begin(0);
      cell != tria.end(0); ++cell)
    if( cell->point_inside (p) )
      return true;

  return false;
}

void check2 ()
{
  Triangulation<3> tria;
  GridIn<3> gridIn;
  gridIn.attach_triangulation (tria);
  std::ifstream inputFile("find_cell_6/grid.inp");
  gridIn.read_ucd (inputFile);

  Point<3> p2(304.767,-57.0113,254.766);
  deallog << inside(tria, p2) << std::endl;
  GridTools::find_active_cell_around_point(tria, p2); //OK

  int idx=0;
  for (Triangulation<3>::active_cell_iterator  cell = tria.begin_active();  cell!=tria.end();  ++cell, ++idx)
    {
      if (idx==21)
	cell->set_refine_flag();      
    }
  tria.execute_coarsening_and_refinement ();

  deallog << inside(tria, p2) << std::endl;

  GridTools::find_active_cell_around_point(tria, p2); //triggered exception
}

void check1 ()
{
  Triangulation<3> tria;
  GridGenerator::hyper_cube (tria);
  tria.refine_global (3);

  for (int i=0;i<3;++i)
    {
      for (int j=0;j<1000;++j)
	{
	  Point<3> p ((rand()%1000)/1000.0,(rand()%1000)/1000.0,(rand()%1000)/1000.0);
	  if (!inside(tria, p))
	    deallog << "NOT INSIDE" << std::endl;
	  GridTools::find_active_cell_around_point(tria, p);
	}

      for(Triangulation<3>::active_cell_iterator cell = tria.begin_active();
	  cell != tria.end(); ++cell)
	for (unsigned int f=0; f<GeometryInfo<3>::faces_per_cell; ++f)
	  {
	    if (cell->face(f)->at_boundary() && (rand()%5)==1)
	      cell->set_refine_flag();
	  }

      tria.execute_coarsening_and_refinement();
    } 
}

int main () 
{
  std::ofstream logfile("find_cell_6/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  try
    {
//      check1();
      check2();
    }
  catch (const std::exception &exc)
    {
				       // we shouldn't get here...
      deallog << "Caught an error..." << std::endl;
      deallog << exc.what() << std::endl;
    }
}

  
  
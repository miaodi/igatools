//-+--------------------------------------------------------------------
// Igatools a general purpose Isogeometric analysis library.
// Copyright (C) 2012-2016  by the igatools authors (see authors.txt).
//
// This file is part of the igatools library.
//
// The igatools library is free software: you can use it, redistribute
// it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//-+--------------------------------------------------------------------

/*
 *  Test for the SphericalFunction class in writer
 *
 *  author: pauletti
 *  date: 2014-10-24
 *
 */

#include "../tests.h"

#include <igatools/base/quadrature_lib.h>
#include <igatools/functions/grid_function_lib.h>

#include <igatools/io/writer.h>


template <int dim>
void write_sphere()
{
  OUTSTART

  using Function = grid_functions::SphereGridFunction<dim>;

  auto grid = Grid<dim>::const_create();

  auto F = Function::const_create(grid);

  const int codim = 1;
  Writer<dim, codim> writer(F, 4);
  writer.save("sphere_dim" + std::to_string(dim), "ascii");
  writer.save("sphere_dim" + std::to_string(dim) + "_bin", "appended");
  writer.print_info(out);

  OUTEND
}


int main()
{
  out.depth_console(10);

  write_sphere<1>();
  write_sphere<2>();

  return 0;
}

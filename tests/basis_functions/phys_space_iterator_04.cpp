//-+--------------------------------------------------------------------
// Igatools a general purpose Isogeometric analysis library.
// Copyright (C) 2012-2015  by the igatools authors (see authors.txt).
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
 *  Test for the evaluation of physical space basis functions
 *  values and gradients the cylindrical mapping.
 *
 *  author:
 *  date:
 *
 */

#include "../tests.h"

#include <igatools/base/quadrature_lib.h>
#include <igatools/functions/function_lib.h>
#include <igatools/functions/identity_function.h>
#include <igatools/basis_functions/bspline_space.h>
#include <igatools/basis_functions/physical_space.h>
#include <igatools/basis_functions/physical_space_element.h>
#include <igatools/basis_functions/phys_space_element_handler.h>


template<int dim, int codim=0>
auto
create_function(shared_ptr<Grid<dim>> grid)
{
  using Function = functions::CylindricalAnnulus<dim>;
  auto map = Function::const_create(grid, IdentityFunction<dim>::const_create(grid),
                              1.0, 2.0, 0.0, 1.0, 0.0, numbers::PI/3.0);
  return map;
}

template <int dim, int order = 0, int range=1, int rank=1, int codim = 0>
void elem_values(const int n_knots = 2, const int deg=1)
{

  const int k = dim;
  using BspSpace = BSplineSpace<dim, range, rank>;
  using Space = PhysicalSpace<dim,range,rank,codim, Transformation::h_grad>;

  auto grid  = Grid<dim>::const_create(n_knots);

  auto ref_space = BspSpace::const_create(deg, grid);
  auto map_func = create_function(grid);

  auto space = Space::const_create(ref_space, map_func);

  const int n_qp = 2;
  auto quad = QGauss<k>(n_qp);
  auto flag = ValueFlags::value |
              ValueFlags::gradient |
              ValueFlags::hessian |
              ValueFlags::point;

  auto elem_filler = space->create_cache_handler();
  elem_filler->reset(flag, quad);

  auto elem = space->begin();
  auto end = space->end();
  elem_filler->init_element_cache(elem);

  for (; elem != end; ++elem)
  {
    elem_filler->fill_element_cache(elem);

    out << "Basis values: " << endl;
    elem->template get_basis<_Value, k>(0,DofProperties::active).print_info(out);
    out << endl;

    out << "Basis gradients: " << endl;
    elem->template get_basis<_Gradient, k>(0,DofProperties::active).print_info(out);
    out << endl;

    out << "Basis hessians: " << endl;
    elem->template get_basis<_Hessian, k>(0,DofProperties::active).print_info(out);
  }

  out << endl << endl;
}


int main()
{
  out.depth_console(10);

  elem_values<3>();

  return 0;
}

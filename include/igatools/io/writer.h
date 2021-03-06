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

#ifndef WRITER_H_
#define WRITER_H_

#include <igatools/base/config.h>
#include <igatools/geometry/grid.h>
#include <igatools/geometry/domain.h>
#include <igatools/geometry/unit_element.h>
#include <igatools/functions/grid_function_element.h>
#include <igatools/base/quadrature.h>
#include <igatools/functions/function_element.h>

#include <igatools/utils/safe_stl_array.h>

#include <string>

IGA_NAMESPACE_OPEN


template <int dim_, int range_, int rank_ , int codim_>
class PhysicalBasis;


template<int dim, int codim = 0, class T = double>
class Writer
{
public:

  /**
   * Default constructor. Not allowed to be used.
   */
  Writer() = delete;

  /**
   * Copy constructor. Not allowed to be used.
   */
  Writer(const Writer<dim,codim,T> &writer) = delete;


  Writer(const std::shared_ptr<const Grid<dim> > &grid,
         const Index num_points_direction = 2);


  Writer(const std::shared_ptr<const GridFunction<dim,dim+codim>> &grid_function,
         const Index num_points_direction);

  Writer(const std::shared_ptr<const Domain<dim,codim>> &domain,
         const Index num_points_direction);

  /**
   * This constructor builds a Writer object using a distribution for
   * the evaluation points given by the @p quadrature scheme.
   * \note Any field that will be added to the writer must refer to the same
   * Grid used here, otherwise an exception will be raised.
   * \note The number of points in each coordinate direction must be greater or equal than 2,
   * otherwise an exception will be raised.
   * \see add_field
   */
  Writer(const std::shared_ptr<const Domain<dim,codim>> &domain,
         const std::shared_ptr<const Quadrature<dim>> &quadrature);

  /*
   * Destructor.
   */
  ~Writer() = default;

  /**
   * Assignment operator. Not allowed to be used.
   */
  Writer<dim,codim,T> &operator=(const Writer<dim,codim,T> &writer) = delete;


  /**
   * Save the data on a .vtu file.
   * \param[in] filename - Output file name.
   * \param[in] format - Output format. It can be "ascii" or "appended".
   * \note The .vtu extension should NOT part of the file name.
   */
  void save(const std::string &filename,
            const std::string &format = "ascii") const;


  /**
   * Writes the vtu into a LogStream, filtering it for uniform
   * output across different systems.
   * @note this function is only for testing purposes
   */
  void print_info(LogStream &out) const;


  void add_element_data(const SafeSTLVector<double> &element_data,
                        const std::string &name);

  void add_element_data(const SafeSTLVector<int> &element_data,
                        const std::string &name);

  /**
   * \brief Add data for every evaluation point to the output file.
   * \param[in] n_values_per_point
   * \param[in] type Type of add to be added. It can be scalar, vector
   * or tensor.
   * \param[in] name Name of the field.
   * \param[in] data_iga_elements Data to be added. The different levels of
   * the container are: the first vector level corresponds to the IGA
   * elements; the second one to the evaluation points inside an IGA element;
   * and the third one to the components of the data to be plotted.
   * \note The number of entries of @p data_iga_elements must be equal to the
   * number of IGA elements, otherwise an exception will be raised.
   * \note The number of entries of eachy entry of @p data_iga_elements must
   * be equal to the number of IGA elements, otherwise an exception will be
   * raised.
   * \note The number of values associated to every plot points that are
   * specified in @p data_iga_elements must be equal to @ n_values_per_point,
   * otherwise an exception will be raised.
   */
  void add_point_data(const int n_values_per_point,
                      const std::string &type,
                      const SafeSTLVector<SafeSTLVector<SafeSTLVector<T> > > &data_iga_elements,
                      const std::string &name);


  /**
   * \brief Add a field (of type Function) to the output file.
   */
  template<int range, int rank>
  void add_field(const Function<dim,codim,range,rank> &func,
                 const std::string &name);

  /**
   * \brief Add a field (of type GridFunction) to the output file.
   */
  template<int range>
  void add_field(const GridFunction<dim,range> &func,
                 const std::string &name);


  /**
   * Returns the number of evaluation points used for each IGA (i.e. Bezier) element.
   */
  int get_num_points_per_iga_element() const;

  /**
   * Returns the number of VTK elements used for each IGA element.
   */
  int get_num_vtk_elements_per_iga_element() const;

  /**
   * Returns the number of IGA elements handled by the Writer.
   */
  int get_num_iga_elements() const;

  /**
   * Returns the number of VTK elements handled by the Writer.
   */
  int get_num_vtk_elements() const;

private:


  std::shared_ptr<const Domain<dim,codim> > domain_;

  /**
   * Unit element quadrature rule used for the plot.
   */
  std::shared_ptr< const Quadrature<dim> > quad_plot_;



  const TensorSize<dim> num_points_direction_;


  TensorSize<dim> num_subelements_direction_;

  /**
   * Number of VTK elements contained in each IGA element.
   */
  int n_vtk_elements_per_iga_element_;

  /**
   * Number of VTK elements handled by the Writer.
   */
  int n_vtk_elements_;


  /**
   * Number of IGA elements handled by the Writer.
   */
  const int n_iga_elements_;

  /**
   * Number of evaluation points in each IGA element.
   */
  const int n_points_per_iga_element_;

  /**
   * Number of VTK elements handled by the Writer.
   */
  const int n_vtk_points_;

  unsigned char vtk_element_type_;


  const int sizeof_Real_ = 0;
  std::string string_Real_;

  const int sizeof_int_  = 0;
  std::string string_int_;

  const int sizeof_uchar_  = 0;
  std::string string_uchar_;

  std::stringstream appended_data_;

  int offset_;

  int precision_;

  static const int n_vertices_per_vtk_element_ = UnitElement<dim>::template num_elem<0>();



  /**
   * Specifier for the byte order. It can be "LittleEndian" or "BigEndian".
   * It depends on the architecture on which igatools is built
   * and it is inferred from the installed Boost version.
   */
  std::string byte_order_;


  struct PointData
  {

    PointData(
      const std::string &name,
      const std::string &type,
      const Size num_elements,
      const Size num_points_per_element,
      const Size num_components,
      const std::shared_ptr< const SafeSTLVector<T> > &values)
      :
      name_(name),
      type_(type),
      num_elements_(num_elements),
      num_points_per_element_(num_points_per_element),
      num_components_(num_components),
      values_(values)
    {};

    const std::string name_;

    const std::string type_;

    Size num_elements_;

    Size num_points_per_element_;

    Size num_components_;


    std::shared_ptr< const SafeSTLVector<T> > values_;
  };

  SafeSTLVector< PointData > fields_;


  SafeSTLVector<std::string> names_point_data_scalar_;
  SafeSTLVector<std::string> names_point_data_vector_;
  SafeSTLVector<std::string> names_point_data_tensor_;


  template<class data_type>
  struct CellData
  {
    CellData(
      const SafeSTLVector<data_type> &values,
      const std::string &name)
      :
      values_(new SafeSTLVector<data_type>(values)),
      name_(name),
      type_("scalar"),
      num_components_(1)
    {};

    std::shared_ptr< SafeSTLVector<data_type> > values_;

    const std::string name_;

    const std::string type_;

    iga::Size num_components_;
  };
  SafeSTLVector< CellData<double> > cell_data_double_;
  SafeSTLVector< CellData<int> > cell_data_int_;


  SafeSTLVector<std::string> names_cell_data_scalar_;
  SafeSTLVector<std::string> names_cell_data_vector_;
  SafeSTLVector<std::string> names_cell_data_tensor_;



  void fill_points_and_connectivity(
    SafeSTLVector<SafeSTLVector<SafeSTLArray<T,3> > > &points_in_iga_elements,
    SafeSTLVector<SafeSTLVector<SafeSTLArray<int,n_vertices_per_vtk_element_> > >
    &vtk_elements_connectivity) const;

  void get_subelements(
    const DomainElement<dim,codim> &elem,
    const int elem_flat_id,
    SafeSTLVector< SafeSTLArray<int,n_vertices_per_vtk_element_> > &vtk_elements_connectivity,
    SafeSTLVector< SafeSTLArray<T,3> > &points_phys_iga_element) const;



  template<class Out>
  void save_ascii(Out &file,
                  const SafeSTLVector<SafeSTLVector<SafeSTLArray<T,3> > > &points_in_iga_elements,
                  const SafeSTLVector<SafeSTLVector<SafeSTLArray<int,n_vertices_per_vtk_element_> > >
                  &vtk_elements_connectivity) const;


  void save_appended(const std::string &filename,
                     const SafeSTLVector<SafeSTLVector<SafeSTLArray<T,3> > > &points_in_iga_elements,
                     const SafeSTLVector<SafeSTLVector<SafeSTLArray< int,n_vertices_per_vtk_element_> > >
                     &vtk_elements_connectivity) const;



};


using std::string;
using std::shared_ptr;

template<int dim, int codim, class T>
template<int range, int rank>
inline
void
Writer<dim, codim, T>::
add_field(const Function<dim,codim,range,rank> &func,
          const string &name)
{
  Assert(domain_ == func.get_domain(),
         ExcMessage("Different domains between the function and the Writer."));

  //--------------------------------------------------------------------------
  static_assert(range <= 3,"The maximum allowed physical domain for VTK file is 3.");
  //--------------------------------------------------------------------------


  //--------------------------------------------------------------------------
  // get the fields to write and assign them to the vtkUnstructuredGrid object
  using function_element::Flags;
  using _D0 = function_element::template _D<0>;

  auto func_cache_handler = func.create_cache_handler();
  func_cache_handler->template set_flags<dim>(Flags::D0);

  auto f_elem = func.cbegin();
  const auto f_end  = func.cend();



  func_cache_handler->init_cache(*f_elem,quad_plot_);


  const auto n_elements = domain_->get_grid_function()->get_grid()->get_num_all_elems();
  const auto n_pts_per_elem = quad_plot_->get_num_points();

  const int n_values_per_pt = (range == 1 ? 1 : std::pow(range, rank)) ;
  auto data_ptr = std::make_shared<SafeSTLVector<T>>(n_elements * n_pts_per_elem * n_values_per_pt);
  auto &data = *data_ptr;
  if (rank == 0 || (rank == 1 && range == 1))
  {
    int pos = 0;
    for (; f_elem != f_end; ++f_elem)
    {
      func_cache_handler->template fill_cache<dim>(*f_elem,0);

      const auto &field_values = f_elem->template get_values_from_cache<_D0,dim>(0);

      for (int iPt = 0; iPt < n_pts_per_elem; ++iPt)
        data[pos++] = field_values[iPt][0];
    }

    fields_.emplace_back(PointData(name,"scalar",n_elements,n_pts_per_elem, n_values_per_pt, data_ptr));
    names_point_data_scalar_.emplace_back(name);
  }
  else if (rank == 1 && range > 1)
  {
    int pos = 0;
    for (; f_elem != f_end; ++f_elem)
    {
      func_cache_handler->template fill_cache<dim>(*f_elem,0);

      const auto &field_values = f_elem->template get_values_from_cache<_D0,dim>(0);

      for (int iPt = 0; iPt < n_pts_per_elem; ++iPt)
      {
        const auto &field_value_ipt = field_values[iPt];
        for (int i = 0; i < range; ++i)
          data[pos++] = field_value_ipt[i];
      }
    }

    fields_.emplace_back(PointData(name,"vector",n_elements,n_pts_per_elem, n_values_per_pt, data_ptr));
    names_point_data_vector_.emplace_back(name);
  }
  else if (rank == 2)
  {
    Assert(false,ExcNotImplemented());
    int pos = 0;
    for (; f_elem != f_end; ++f_elem)
    {
      // TODO (pauletti, Sep 12, 2014): fix next line
      Assert(true, ExcMessage(" fix next line "));
      func_cache_handler->template fill_cache<dim>(*f_elem,0);

      const auto &field_values = f_elem->template get_values_from_cache<_D0,dim>(0);

      for (int iPt = 0; iPt < n_pts_per_elem; ++iPt)
      {
        const auto &field_value_ipt = field_values[ iPt ];
        for (int i = 0; i < range; ++i)
        {
          const auto &field_value_ipt_i = field_value_ipt[i];

          for (int j = 0; j < range; ++j)
            data[pos++] = field_value_ipt_i[j];
        }
      }
    }

    fields_.emplace_back(PointData(name,"tensor",n_elements,n_pts_per_elem,n_values_per_pt,data_ptr));
    names_point_data_tensor_.emplace_back(name);
  }

  //--------------------------------------------------------------------------
//#endif
}





template<int dim, int codim, class T>
template<int range>
inline
void
Writer<dim, codim, T>::
add_field(const GridFunction<dim,range> &func,
          const string &name)
{
  const auto grid = domain_->get_grid_function()->get_grid();
  Assert(grid == func.get_grid(),
         ExcMessage("Different grids between the function and the Writer."));

  //--------------------------------------------------------------------------
  static_assert(range <= 3,"The maximum allowed physical domain for VTK file is 3.");
  //--------------------------------------------------------------------------

  //--------------------------------------------------------------------------
  // get the fields to write and assign them to the vtkUnstructuredGrid object
  using grid_function_element::Flags;
  using _D0 = grid_function_element::_D<0>;

  auto func_cache_handler = func.create_cache_handler();
  func_cache_handler->template set_flags<dim>(Flags::D0);

  auto f_elem = func.cbegin();
  const auto f_end  = func.cend();



  func_cache_handler->init_cache(*f_elem,quad_plot_);


  const auto n_elements = grid->get_num_all_elems();
  const auto n_pts_per_elem = quad_plot_->get_num_points();

  const int n_values_per_pt = range;
  auto data_ptr = std::make_shared<SafeSTLVector<T>>(n_elements * n_pts_per_elem * n_values_per_pt);
  auto &data = *data_ptr;
  if (range == 1)
  {
    int pos = 0;
    for (; f_elem != f_end; ++f_elem)
    {
      func_cache_handler->template fill_cache<dim>(*f_elem,0);

      const auto &field_values = f_elem->template get_values_from_cache<_D0,dim>(0);

      for (int iPt = 0; iPt < n_pts_per_elem; ++iPt)
        data[pos++] = field_values[iPt][0];
    }

    fields_.emplace_back(PointData(name,"scalar",n_elements,n_pts_per_elem, n_values_per_pt, data_ptr));
    names_point_data_scalar_.emplace_back(name);
  }
  else if (range > 1)
  {
    int pos = 0;
    for (; f_elem != f_end; ++f_elem)
    {
      func_cache_handler->template fill_cache<dim>(*f_elem,0);

      const auto &field_values = f_elem->template get_values_from_cache<_D0,dim>(0);

      for (int iPt = 0; iPt < n_pts_per_elem; ++iPt)
      {
        const auto &field_value_ipt = field_values[iPt];
        for (int i = 0; i < range; ++i)
          data[pos++] = field_value_ipt[i];
      }
    }

    fields_.emplace_back(PointData(name,"vector",n_elements,n_pts_per_elem, n_values_per_pt, data_ptr));
    names_point_data_vector_.emplace_back(name);
  }
  //--------------------------------------------------------------------------
//#endif
}





IGA_NAMESPACE_CLOSE

#endif /* WRITER_H_ */

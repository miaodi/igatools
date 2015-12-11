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

#include <igatools/io/objects_container_writer.h>

#ifdef XML_IO

#include <igatools/base/objects_container.h>
#include <igatools/utils/safe_stl_set.h>

#include <igatools/io/xml_document.h>
#include <igatools/io/xml_element.h>

#include <igatools/geometry/grid.h>
#include <igatools/geometry/grid_function_lib.h>
#include <igatools/basis_functions/spline_space.h>
#include <igatools/basis_functions/bspline.h>
#include <igatools/basis_functions/nurbs.h>
#include <igatools/functions/function.h>
#include <igatools/functions/ig_function.h>
#include <igatools/functions/function_lib.h>

#include <boost/fusion/algorithm/iteration/for_each.hpp>


using std::string;
//using std::to_string;
using std::shared_ptr;
using std::remove_reference;
using std::dynamic_pointer_cast;
using std::const_pointer_cast;

IGA_NAMESPACE_OPEN

void
ObjectsContainerWriter::
write(const string &file_path,
      const shared_ptr<ObjectsContainer> container)
{
    // Copying the objects container and filling it with all its dependencies.
    const auto full_container = shared_ptr<ObjectsContainer>(new
      ObjectsContainer(*container));
    full_container->fill_not_inserted_dependencies();

    const auto xml_doc = XMLDocument::create_void_document("Igatools");
    const auto igatools_elem = xml_doc->get_document_element();
    igatools_elem->add_attribute(string("FormatVersion"), string("2.0"));

    Self_::write_grids(full_container, xml_doc);
    Self_::write_spline_spaces(full_container, xml_doc);
    Self_::write_reference_space_bases(full_container, xml_doc);
    Self_::write_grid_functions(full_container, xml_doc);
    Self_::write_domains(full_container, xml_doc);
    Self_::write_physical_space_bases(full_container, xml_doc);
    Self_::write_functions(full_container, xml_doc);

    xml_doc->write_to_file (file_path);
}



void
ObjectsContainerWriter::
write_grids (const shared_ptr<ObjectsContainer> container,
             const XMLDocPtr_ xml_doc)
{
  using GridPtrs = typename ObjectsContainer::GridPtrs;

  GridPtrs valid_grid_ptr_types;
  for_each(valid_grid_ptr_types, [&](const auto &ptr_type)
  {
    using Type = typename remove_reference<decltype(ptr_type)>::type::element_type;

    for (const auto &id : container->template get_object_ids<Type>())
        Self_::write_grid<Type>(container->template get_object<Type>(id),
                                xml_doc);

    for (const auto &id : container->template get_const_object_ids<Type>())
        Self_::write_grid<const Type>(container->template get_const_object<Type>(id),
                                      xml_doc);
  });
}



void
ObjectsContainerWriter::
write_spline_spaces (const shared_ptr<ObjectsContainer> container,
             const XMLDocPtr_ xml_doc)
{
  using SpSpacePtrs = typename ObjectsContainer::SpSpacePtrs;

  SpSpacePtrs valid_spline_space_ptr_types;
  for_each(valid_spline_space_ptr_types, [&](const auto &ptr_type)
  {
    using Type = typename remove_reference<decltype(ptr_type)>::type::element_type;

    for (const auto &id : container->template get_object_ids<Type>())
        Self_::write_spline_space<Type>(container->template get_object<Type>(id),
                                        xml_doc);

    for (const auto &id : container->template get_const_object_ids<Type>())
        Self_::write_spline_space<const Type>(container->template get_const_object<Type>(id),
                                              xml_doc);
  });
}



void
ObjectsContainerWriter::
write_reference_space_bases (const shared_ptr<ObjectsContainer> container,
             const XMLDocPtr_ xml_doc)
{
  using RefSpacePtrs = typename ObjectsContainer::RefSpacePtrs;

  RefSpacePtrs valid_ref_space_ptr_types;
  for_each(valid_ref_space_ptr_types, [&](const auto &ptr_type)
  {
    using Type = typename remove_reference<decltype(ptr_type)>::type::element_type;

    static const int dim = Type::dim;
    static const int range = Type::range;
    static const int rank = Type::rank;

    using NURBSType = NURBS<dim, range, rank>;
    using BSplineType = typename NURBSType::BSpBasis;

    for (const auto &id : container->template get_object_ids<Type>())
    {
        const auto ref_space = container->template get_object<Type>(id);

        const auto bs_space = dynamic_pointer_cast<BSplineType>(ref_space);
        const auto nr_space = dynamic_pointer_cast<NURBSType>(ref_space);

#ifndef NDEBUG
        AssertThrow (bs_space != nullptr || nr_space != nullptr,
                ExcMessage("Invalid reference space type."));
#endif

        if (bs_space != nullptr)
            Self_::write_bspline<BSplineType>(bs_space, xml_doc);
        else
            Self_::write_bspline<NURBSType>(nr_space, xml_doc);
    }

    for (const auto &id : container->template get_const_object_ids<Type>())
    {
        const auto ref_space = container->template get_const_object<Type>(id);

        const auto bs_space = dynamic_pointer_cast<const BSplineType>(ref_space);
        const auto nr_space = dynamic_pointer_cast<const NURBSType>(ref_space);

#ifndef NDEBUG
        AssertThrow (bs_space != nullptr || nr_space != nullptr,
                ExcMessage("Invalid reference space type."));
#endif

        if (bs_space != nullptr)
            Self_::write_bspline<const BSplineType>(bs_space, xml_doc);
        else
            Self_::write_bspline<const NURBSType>(nr_space, xml_doc);
    }
  });
}



void
ObjectsContainerWriter::
write_grid_functions (const shared_ptr<ObjectsContainer> container,
                      const XMLDocPtr_ xml_doc)
{
  using GridFuncPtrs = typename ObjectsContainer::GridFuncPtrs;

  GridFuncPtrs valid_gf_ptr_types;
  for_each(valid_gf_ptr_types, [&](const auto &ptr_type)
  {
    using Type = typename remove_reference<decltype(ptr_type)>::type::element_type;

    static const int dim = Type::dim;
    static const int space_dim = Type::space_dim;

    using IdGridFunc = grid_functions::IdentityGridFunction<dim>;
    using LinearGridFunc = grid_functions::LinearGridFunction<dim, space_dim>;
    using ConstantGridFunc = grid_functions::ConstantGridFunction<dim, space_dim>;
    using IgGridFunc = IgGridFunction<dim, space_dim>;

    for (const auto &id : container->template get_object_ids<Type>())
    {
        const auto grid_func = container->template get_object<Type>(id);

        const auto id_f = dynamic_pointer_cast<IdGridFunc>(grid_func);
        const auto li_f = dynamic_pointer_cast<LinearGridFunc>(grid_func);
        const auto ct_f = dynamic_pointer_cast<ConstantGridFunc>(grid_func);
        const auto ig_f = dynamic_pointer_cast<IgGridFunc>(grid_func);

#ifndef NDEBUG
        AssertThrow (id_f != nullptr || li_f != nullptr ||
                     ct_f != nullptr || ig_f != nullptr,
                ExcMessage("Invalid grid function type."));
#endif

        if (id_f != nullptr)
            Self_::write_identity_grid_function<IdGridFunc>(id_f, xml_doc);
        else if (li_f != nullptr)
            Self_::write_linear_grid_function<LinearGridFunc>(li_f, xml_doc);
        else if (ct_f != nullptr)
            Self_::write_constant_grid_function<ConstantGridFunc>(ct_f, xml_doc);
        else
            Self_::write_ig_grid_function<IgGridFunc>(ig_f, xml_doc);
    }

    for (const auto &id : container->template get_const_object_ids<Type>())
    {
        const auto grid_func = container->template get_const_object<Type>(id);

        const auto id_f = dynamic_pointer_cast<const IdGridFunc>(grid_func);
        const auto li_f = dynamic_pointer_cast<const LinearGridFunc>(grid_func);
        const auto ct_f = dynamic_pointer_cast<const ConstantGridFunc>(grid_func);
        const auto ig_f = dynamic_pointer_cast<const IgGridFunc>(grid_func);

#ifndef NDEBUG
        AssertThrow (id_f != nullptr || li_f != nullptr ||
                     ct_f != nullptr || ig_f != nullptr,
                ExcMessage("Invalid grid function type."));
#endif

        if (id_f != nullptr)
            Self_::write_identity_grid_function<const IdGridFunc>(id_f, xml_doc);
        else if (li_f != nullptr)
            Self_::write_linear_grid_function<const LinearGridFunc>(li_f, xml_doc);
        else if (ct_f != nullptr)
            Self_::write_constant_grid_function<const ConstantGridFunc>(ct_f, xml_doc);
        else
            Self_::write_ig_grid_function<const IgGridFunc>(ig_f, xml_doc);
    }
  });
}



void
ObjectsContainerWriter::
write_domains (const shared_ptr<ObjectsContainer> container,
               const XMLDocPtr_ xml_doc)
{
  using DomainPtrs = typename ObjectsContainer::DomainPtrs;

  DomainPtrs valid_dm_ptr_types;
  for_each(valid_dm_ptr_types, [&](const auto &ptr_type)
  {
    using Type = typename remove_reference<decltype(ptr_type)>::type::element_type;

    for (const auto &id : container->template get_object_ids<Type>())
        Self_::write_domain<Type>(container->template get_object<Type>(id),
                                  xml_doc);

    for (const auto &id : container->template get_const_object_ids<Type>())
        Self_::write_domain<const Type>(container->template get_const_object<Type>(id),
                                        xml_doc);
  });
}



void
ObjectsContainerWriter::
write_physical_space_bases (const shared_ptr<ObjectsContainer> container,
                            const XMLDocPtr_ xml_doc)
{
  using PhysSpacePtrs = typename ObjectsContainer::PhysSpacePtrs;

  PhysSpacePtrs valid_ps_ptr_types;
  for_each(valid_ps_ptr_types, [&](const auto &ptr_type)
  {
    using Type = typename remove_reference<decltype(ptr_type)>::type::element_type;

    for (const auto &id : container->template get_object_ids<Type>())
        Self_::write_phys_space_basis<Type>(container->template get_object<Type>(id),
                                            xml_doc);

    for (const auto &id : container->template get_const_object_ids<Type>())
        Self_::write_phys_space_basis<const Type>(container->template get_const_object<Type>(id),
                                                  xml_doc);
  });
}



void
ObjectsContainerWriter::
write_functions (const shared_ptr<ObjectsContainer> container,
                 const XMLDocPtr_ xml_doc)
{
  using FuncPtrs = typename ObjectsContainer::FunctionPtrs;

  FuncPtrs valid_f_ptr_types;
  for_each(valid_f_ptr_types, [&](const auto &ptr_type)
  {
    using Type = typename remove_reference<decltype(ptr_type)>::type::element_type;

    static const int dim = Type::dim;
    static const int codim = Type::codim;
    static const int range = Type::range;
    static const int rank = Type::rank;

    using LinearFunc = functions::LinearFunction<dim, codim, range>;
    using ConstantFunc = functions::ConstantFunction<dim, codim, range, rank>;
    using IgFunc = IgFunction<dim, codim, range, rank>;

    for (const auto &id : container->template get_object_ids<Type>())
    {
        const auto func = container->template get_object<Type>(id);

        const auto li_f = dynamic_pointer_cast<LinearFunc>(func);
        const auto ct_f = dynamic_pointer_cast<ConstantFunc>(func);
        const auto ig_f = dynamic_pointer_cast<IgFunc>(func);

#ifndef NDEBUG
        AssertThrow (li_f != nullptr || ct_f != nullptr || ig_f != nullptr,
                ExcMessage("Invalid function type."));
#endif

        if (li_f != nullptr)
            Self_::write_linear_function<LinearFunc>(li_f, xml_doc);
        else if (ct_f != nullptr)
            Self_::write_constant_function<ConstantFunc>(ct_f, xml_doc);
        else
            Self_::write_ig_function<IgFunc>(ig_f, xml_doc);
    }

    for (const auto &id : container->template get_const_object_ids<Type>())
    {
        const auto func = container->template get_const_object<Type>(id);

        const auto li_f = dynamic_pointer_cast<const LinearFunc>(func);
        const auto ct_f = dynamic_pointer_cast<const ConstantFunc>(func);
        const auto ig_f = dynamic_pointer_cast<const IgFunc>(func);

#ifndef NDEBUG
        AssertThrow (li_f != nullptr || ct_f != nullptr || ig_f != nullptr,
                ExcMessage("Invalid function type."));
#endif

        if (li_f != nullptr)
            Self_::write_linear_function<const LinearFunc>(li_f, xml_doc);
        else if (ct_f != nullptr)
            Self_::write_constant_function<const ConstantFunc>(ct_f, xml_doc);
        else
            Self_::write_ig_function<const IgFunc>(ig_f, xml_doc);
    }
  });
}



template <class Grid>
void
ObjectsContainerWriter::
write_grid (const shared_ptr<Grid> grid,
            const XMLDocPtr_ xml_doc)
{
    const auto obj_elem = xml_doc->create_new_element("Grid");

    static const int dim = Grid::dim;

    obj_elem->add_attribute("LocalObjectId", grid->get_object_id());
    obj_elem->add_attribute("Dim", dim);

    for (int dir = 0; dir < dim; ++dir)
    {
        const auto &knt_coord = grid->get_knot_coordinates(dir);
        const auto knot_elem = xml_doc->create_size_dir_vector_element(
                "Knots", knt_coord, dir);
        obj_elem->append_child_element(knot_elem);
    }

    const auto igt_elem = xml_doc->get_document_element();
    igt_elem->append_child_element(obj_elem);
}



template <class SpSpace>
void
ObjectsContainerWriter::
write_spline_space (const shared_ptr<SpSpace> spline_space,
                    const XMLDocPtr_ xml_doc)
{
    const auto obj_elem = xml_doc->create_new_element("SplineSpace");

    static const int dim    = SpSpace::dim;
    static const int range  = SpSpace::range;
    static const int rank   = SpSpace::rank;
    static const int n_comp = SpSpace::n_components;

    obj_elem->add_attribute("LocalObjectId", spline_space->get_object_id());
    obj_elem->add_attribute("Dim", dim);
    obj_elem->add_attribute("Range", range);
    obj_elem->add_attribute("Rank", rank);

    const auto &degrees_table = spline_space->get_degree_table();
    const auto &mults_table = spline_space->get_interior_mult();
    const auto &period_table = spline_space->get_periodic_table();

    const auto grid_elem = xml_doc->create_new_element("Grid");
    grid_elem->add_attribute("GetFromLocalObjectId",
                            spline_space->get_grid()->get_object_id());
    obj_elem->append_child_element(grid_elem);

    const auto ssp_comps = xml_doc->create_new_element("SplineSpaceComponents");

    for (int comp_id = 0; comp_id < n_comp; ++comp_id)
    {
        const auto ssp_comp = xml_doc->create_new_element("SplineSpaceComponent");
        ssp_comp->add_attribute("ComponentId", comp_id);

        // Writing degrees.
        const auto degrees = degrees_table[comp_id];
        SafeSTLVector<Index> degrees_vec (degrees.size());
        auto it_deg = degrees_vec.begin();
        for (const auto &d : degrees)
            *it_deg++ = d;
        const auto degrees_xml = xml_doc->
                create_vector_element("Degrees", degrees_vec);
        ssp_comp->append_child_element(degrees_xml);


        // Writing interior multiplicities.
        const auto mults   = mults_table[comp_id];

        const auto int_mults = xml_doc->create_new_element("InteriorMultiplicities");
        for (int dir = 0; dir < dim; ++dir)
        {
            const auto &mult_dir = mults.get_data_direction(dir);
            const auto int_mult = xml_doc->
                    create_size_dir_vector_element("InteriorMultiplicities",
                                                   mult_dir, dir);
            int_mults->append_child_element(int_mult);
        }
        ssp_comp->append_child_element(int_mults);


        // Writing periodicity.
        const auto period  = period_table[comp_id];
        SafeSTLVector<bool> period_vec (period.size());
        auto it_per = period_vec.begin();
        for (const auto &p : period)
            *it_per++ = p;

        const auto period_xml = xml_doc->
                create_vector_element("Periodicity", period_vec);
        ssp_comp->append_child_element(period_xml);

        ssp_comps->append_child_element(ssp_comp);
    }
    obj_elem->append_child_element(ssp_comps);

    const auto igt_elem = xml_doc->get_document_element();
    igt_elem->append_child_element(obj_elem);
}



template <class BSpline>
void
ObjectsContainerWriter::
write_bspline (const shared_ptr<BSpline> bspline,
               const XMLDocPtr_ xml_doc)
{
    const auto obj_elem = xml_doc->create_new_element("BSpline");

    static const int dim    = BSpline::dim;
    static const int range  = BSpline::range;
    static const int rank   = BSpline::rank;
    static const int n_comp = BSpline::n_components;

    obj_elem->add_attribute("LocalObjectId", bspline->get_object_id());
    obj_elem->add_attribute("Dim", dim);
    obj_elem->add_attribute("Range", range);
    obj_elem->add_attribute("Rank", rank);

    const auto ssp_elem = xml_doc->create_new_element("SplineSpace");
    ssp_elem->add_attribute("GetFromLocalObjectId",
                            bspline->get_spline_space()->get_object_id());
    obj_elem->append_child_element(ssp_elem);

    const auto &end_beh_table = bspline->get_end_behaviour_table();

    const auto end_beh_elem = xml_doc->create_new_element("EndBehaviour");
    for (int comp_id = 0; comp_id < n_comp; ++comp_id)
    {
        SafeSTLVector<string> end_beh_str;
        for (const auto &eb : end_beh_table[comp_id])
        {
            switch (eb)
            {
                case BasisEndBehaviour::interpolatory:
                    end_beh_str.push_back("interpolatory");
                    break;
                case BasisEndBehaviour::end_knots:
                    end_beh_str.push_back("end_knots");
                    break;
                case BasisEndBehaviour::periodic:
                    end_beh_str.push_back("periodic");
                    break;
                default:
                    break;
            }
        }

        const auto end_beh = xml_doc->
                create_vector_element("EndBehaviour", end_beh_str);
        end_beh->add_attribute("ComponentId", comp_id);
        end_beh_elem->append_child_element(end_beh);
    }
    obj_elem->append_child_element(end_beh_elem);


    const auto igt_elem = xml_doc->get_document_element();
    igt_elem->append_child_element(obj_elem);
}



template <class NURBS>
void
ObjectsContainerWriter::
write_nurbs (const shared_ptr<NURBS> nurbs,
             const XMLDocPtr_ xml_doc)
{
    const auto obj_elem = xml_doc->create_new_element("NURBS");

    static const int dim   = NURBS::dim;
    static const int range = NURBS::range;
    static const int rank  = NURBS::rank;

    obj_elem->add_attribute("LocalObjectId", nurbs->get_object_id());
    obj_elem->add_attribute("Dim", dim);
    obj_elem->add_attribute("Range", range);
    obj_elem->add_attribute("Rank", rank);

    const auto bs_elem = xml_doc->create_new_element("BSpline");
    bs_elem->add_attribute("GetFromLocalObjectId",
                            nurbs->get_bspline_basis()->get_object_id());
    obj_elem->append_child_element(bs_elem);

    const auto wf_elem = xml_doc->create_new_element("WeightFunction");
    wf_elem->add_attribute("GetFromLocalObjectId",
                            nurbs->get_weight_func()->get_object_id());
    obj_elem->append_child_element(wf_elem);

    const auto igt_elem = xml_doc->get_document_element();
    igt_elem->append_child_element(obj_elem);
}



template <class IdGridFunc>
void
ObjectsContainerWriter::
write_identity_grid_function (const shared_ptr<IdGridFunc> id_func,
                              const XMLDocPtr_ xml_doc)
{
    const auto obj_elem = xml_doc->create_new_element("IdentityGridFunction");

    static const int dim   = IdGridFunc::dim;

    obj_elem->add_attribute("LocalObjectId", id_func->get_object_id());
    obj_elem->add_attribute("Dim", dim);

    const auto grid_elem = xml_doc->create_new_element("Grid");
    grid_elem->add_attribute("GetFromLocalObjectId",
                            id_func->get_grid()->get_object_id());
    obj_elem->append_child_element(grid_elem);

    const auto igt_elem = xml_doc->get_document_element();
    igt_elem->append_child_element(obj_elem);
}



template <class ConstGridFunc>
void
ObjectsContainerWriter::
write_constant_grid_function (const shared_ptr<ConstGridFunc> const_func,
                              const XMLDocPtr_ xml_doc)
{
    const auto obj_elem = xml_doc->create_new_element("ConstantGridFunction");

    static const int dim         = ConstGridFunc::dim;
    static const int space_dim   = ConstGridFunc::space_dim;

    obj_elem->add_attribute("LocalObjectId", const_func->get_object_id());
    obj_elem->add_attribute("Dim", dim);
    obj_elem->add_attribute("Spacedim", space_dim);

    const auto grid_elem = xml_doc->create_new_element("Grid");
    grid_elem->add_attribute("GetFromLocalObjectId",
                            const_func->get_grid()->get_object_id());
    obj_elem->append_child_element(grid_elem);

    const auto values_vec = const_func->get_constant_value().get_flat_values();
    const auto values_elem = xml_doc->create_vector_element("Values", values_vec);
    obj_elem->append_child_element(values_elem);

    const auto igt_elem = xml_doc->get_document_element();
    igt_elem->append_child_element(obj_elem);
}



template <class LinearGridFunc>
void
ObjectsContainerWriter::
write_linear_grid_function (const shared_ptr<LinearGridFunc> linear_func,
                            const XMLDocPtr_ xml_doc)
{
    const auto obj_elem = xml_doc->create_new_element("LinearGridFunction");

    static const int dim         = LinearGridFunc::dim;
    static const int space_dim   = LinearGridFunc::space_dim;

    obj_elem->add_attribute("LocalObjectId", linear_func->get_object_id());
    obj_elem->add_attribute("Dim", dim);
    obj_elem->add_attribute("Spacedim", space_dim);

    AssertThrow (false, ExcNotImplemented());

    const auto igt_elem = xml_doc->get_document_element();
    igt_elem->append_child_element(obj_elem);
}



template <class IgGridFunc>
void
ObjectsContainerWriter::
write_ig_grid_function (const shared_ptr<IgGridFunc> ig_func,
                        const XMLDocPtr_ xml_doc)
{
    const auto obj_elem = xml_doc->create_new_element("IgGridFunction");

    static const int dim         = IgGridFunc::dim;
    static const int space_dim   = IgGridFunc::space_dim;

    obj_elem->add_attribute("LocalObjectId", ig_func->get_object_id());
    obj_elem->add_attribute("Dim", dim);
    obj_elem->add_attribute("Spacedim", space_dim);

    const auto rb_elem = xml_doc->create_new_element("SplineSpace");
    rb_elem->add_attribute("GetFromLocalObjectId",
                            ig_func->get_basis()->get_object_id());
    obj_elem->append_child_element(rb_elem);

    const auto &coefs = ig_func->get_coefficients();
    const auto coefs_elem = Self_::create_ig_coefs_xml_element(coefs, xml_doc);
    obj_elem->append_child_element(coefs_elem);

    const auto &dofs_prop = ig_func->get_dofs_property();
    const auto dofs_prop_elem = xml_doc->create_new_text_element("DofsProperty", dofs_prop);
    obj_elem->append_child_element(dofs_prop_elem);

    const auto igt_elem = xml_doc->get_document_element();
    igt_elem->append_child_element(obj_elem);
}



template <class Domain>
void
ObjectsContainerWriter::
write_domain (const shared_ptr<Domain> domain,
              const XMLDocPtr_ xml_doc)
{
    const auto obj_elem = xml_doc->create_new_element("Domain");

    static const int dim    = Domain::dim;
    static const int codim  = Domain::space_dim - dim;

    obj_elem->add_attribute("LocalObjectId", domain->get_object_id());
    obj_elem->add_attribute("Dim", dim);
    obj_elem->add_attribute("Codim", codim);

    const auto grid_func_elem = xml_doc->create_new_element("GridFunction");
    grid_func_elem->add_attribute("GetFromLocalObjectId",
                            domain->get_grid_function()->get_object_id());
    obj_elem->append_child_element(grid_func_elem);

    const auto &name = domain->get_name();
    if (name.size() > 0)
    {
        const auto name_elem = xml_doc->create_new_text_element("Name", name);
        obj_elem->append_child_element(name_elem);
    }

    const auto igt_elem = xml_doc->get_document_element();
    igt_elem->append_child_element(obj_elem);
}



template <class PhysSpaceBasis>
void
ObjectsContainerWriter::
write_phys_space_basis (const shared_ptr<PhysSpaceBasis> phys_space,
                        const XMLDocPtr_ xml_doc)
{
    const auto obj_elem = xml_doc->create_new_element("PhysicalSpaceBasis");

    static const int dim   = PhysSpaceBasis::dim;
    static const int range = PhysSpaceBasis::range;
    static const int rank  = PhysSpaceBasis::rank;
    static const int codim = PhysSpaceBasis::codim;

    obj_elem->add_attribute("LocalObjectId", phys_space->get_object_id());
    obj_elem->add_attribute("Dim", dim);
    obj_elem->add_attribute("Range", range);
    obj_elem->add_attribute("Rank", rank);
    obj_elem->add_attribute("Codim", codim);

    const auto rb_elem = xml_doc->create_new_element("ReferenceSpaceBasis");
    rb_elem->add_attribute("GetFromLocalObjectId",
                            phys_space->get_reference_basis()->get_object_id());
    obj_elem->append_child_element(rb_elem);

    const auto dm_elem = xml_doc->create_new_element("Domain");
    dm_elem->add_attribute("GetFromLocalObjectId",
                            phys_space->get_physical_domain()->get_object_id());
    obj_elem->append_child_element(dm_elem);

    const auto trans = phys_space->get_transformation_type();
    string trans_str = "";
    switch (trans)
    {
        case Transformation::h_grad:
            trans_str = "h_grad";
            break;
        case Transformation::h_curl:
            trans_str = "h_curl";
            break;
        case Transformation::h_div:
            trans_str = "h_div";
            break;
        case Transformation::l_2:
            trans_str = "l_2";
            break;
        default:
            break;
    }
    const auto tr_elem = xml_doc->create_new_text_element("Transformation", trans_str);
    obj_elem->append_child_element(tr_elem);

    const auto igt_elem = xml_doc->get_document_element();
    igt_elem->append_child_element(obj_elem);
}



template <class IgFunction>
void
ObjectsContainerWriter::
write_ig_function (const shared_ptr<IgFunction> ig_function,
                   const XMLDocPtr_ xml_doc)
{
    const auto obj_elem = xml_doc->create_new_element("IgFunction");

    static const int dim   = IgFunction::dim;
    static const int range = IgFunction::range;
    static const int rank  = IgFunction::rank;
    static const int codim = IgFunction::codim;

    obj_elem->add_attribute("LocalObjectId", ig_function->get_object_id());
    obj_elem->add_attribute("Dim", dim);
    obj_elem->add_attribute("Range", range);
    obj_elem->add_attribute("Rank", rank);
    obj_elem->add_attribute("Codim", codim);

    const auto ps_elem = xml_doc->create_new_element("PhysicalSpaceBasis");
    ps_elem->add_attribute("GetFromLocalObjectId",
                            ig_function->get_basis()->get_object_id());
    obj_elem->append_child_element(ps_elem);

    const auto &coefs = ig_function->get_coefficients();
    const auto coefs_elem = Self_::create_ig_coefs_xml_element(coefs, xml_doc);
    obj_elem->append_child_element(coefs_elem);

    const auto &name = ig_function->get_name();
    if (name.size() > 0)
    {
        const auto name_elem = xml_doc->create_new_text_element("Name", name);
        obj_elem->append_child_element(name_elem);
    }

    const auto &dofs_prop = ig_function->get_dofs_property();
    const auto dofs_prop_elem = xml_doc->create_new_text_element("DofsProperty", dofs_prop);
    obj_elem->append_child_element(dofs_prop_elem);

    const auto igt_elem = xml_doc->get_document_element();
    igt_elem->append_child_element(obj_elem);
}



template <class ConstantFunction>
void
ObjectsContainerWriter::
write_constant_function (const shared_ptr<ConstantFunction> const_function,
                         const XMLDocPtr_ xml_doc)
{
    const auto obj_elem = xml_doc->create_new_element("ConstantFunction");

    static const int dim   = ConstantFunction::dim;
    static const int range = ConstantFunction::range;
    static const int rank  = ConstantFunction::rank;
    static const int codim = ConstantFunction::codim;

    obj_elem->add_attribute("LocalObjectId", const_function->get_object_id());
    obj_elem->add_attribute("Dim", dim);
    obj_elem->add_attribute("Range", range);
    obj_elem->add_attribute("Rank", rank);
    obj_elem->add_attribute("Codim", codim);

    const auto dm_elem = xml_doc->create_new_element("Domain");
    dm_elem->add_attribute("GetFromLocalObjectId",
                            const_function->get_domain()->get_object_id());
    obj_elem->append_child_element(dm_elem);

    const auto values_vec = const_function->get_constant_value().get_flat_values();
    const auto values_elem = xml_doc->create_vector_element("Values", values_vec);
    obj_elem->append_child_element(values_elem);

    const auto igt_elem = xml_doc->get_document_element();
    igt_elem->append_child_element(obj_elem);
}



template <class LinearFunction>
void
ObjectsContainerWriter::
write_linear_function (const shared_ptr<LinearFunction> linear_function,
                       const XMLDocPtr_ xml_doc)
{
    const auto obj_elem = xml_doc->create_new_element("LinearFunction");

    static const int dim   = LinearFunction::dim;
    static const int range = LinearFunction::range;
    static const int rank  = LinearFunction::rank;
    static const int codim = LinearFunction::codim;

    obj_elem->add_attribute("LocalObjectId", linear_function->get_object_id());
    obj_elem->add_attribute("Dim", dim);
    obj_elem->add_attribute("Range", range);
    obj_elem->add_attribute("Rank", rank);
    obj_elem->add_attribute("Codim", codim);

    AssertThrow (false, ExcNotImplemented());

    const auto igt_elem = xml_doc->get_document_element();
    igt_elem->append_child_element(obj_elem);
}



shared_ptr<XMLElement>
ObjectsContainerWriter::
create_ig_coefs_xml_element(const IgCoefficients &coefs,
                            const XMLDocPtr_ xml_doc)
{
    const auto ic_elem = xml_doc->create_new_element("IgCoefficients");
    ic_elem->add_attribute("Size", coefs.size());
//    <IgCoefficients Size="12">
//      <Indices>
//        0 1 2 3 4 5 6 7 8 9 10 11
//      </Indices>
//      <Values>
//        1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0
//      </Values>
//    </IgCoefficients>

    SafeSTLVector<Index> indices (coefs.size());
    SafeSTLVector<Real> values (coefs.size());
    auto it_ind = indices.begin();
    auto it_val = values.begin();
    for (const auto &it : coefs)
    {
        *it_ind++ = it.first;
        *it_val++ = it.second;
    }

    const auto indices_xml = xml_doc->create_vector_element("Indices", indices);
    const auto values_xml  = xml_doc->create_vector_element("Values",  values);

    ic_elem->append_child_element(indices_xml);
    ic_elem->append_child_element(values_xml);

    return ic_elem;
}

IGA_NAMESPACE_CLOSE

#endif // XML_IO

//-+--------------------------------------------------------------------
// Igatools a general purpose Isogeometric analysis library.
// Copyright (C) 2012-2014  by the igatools authors (see authors.txt).
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

#ifndef __NURBS_SPACE_H_
#define __NURBS_SPACE_H_

#include <igatools/base/config.h>
#include <igatools/basis_functions/bspline_space.h>

IGA_NAMESPACE_OPEN

//Forward declaration to avoid including the header
template < int, int, int > class NURBSElementAccessor;

/**
 * Multivariate (tensor product) scalar, vector or k-tensor
 * valued NURBS space.
 *
 * @ingroup containers
 */
template <int dim_, int range_ = 1, int rank_ = 1>
class NURBSSpace :
    public std::enable_shared_from_this<NURBSSpace<dim_,range_,rank_> >,
    public FunctionSpaceOnGrid<CartesianGrid<dim_> >

{
private:
    using BaseSpace = FunctionSpaceOnGrid<CartesianGrid<dim_>>;
    using self_t = NURBSSpace<dim_, range_, rank_>;
    using spline_space_t = BSplineSpace<dim_, range_, rank_>;

public:
    /** see documentation in \ref FunctionSpaceOnGrid */
    using PushForwardType = PushForward<Transformation::h_grad,dim_,0>;

    using RefSpace = self_t;

    using GridType = typename PushForwardType::GridType;

    static const int dim = PushForwardType::dim;

    static const int codim = PushForwardType::codim;

    static const int space_dim = PushForwardType::space_dim;

    static const int range = spline_space_t::range;

    static const int rank = spline_space_t::rank;

    static const iga::RefSpaceType ref_space_type = iga::RefSpaceType(1);

    static constexpr int n_components = spline_space_t::n_components;

    static const std::array<int, n_components> components;

    static const bool has_weights = true;

    static const std::array<int, dim> dims;



public:
    using Func = typename spline_space_t::Func;
    template <int order>
    using Derivative = typename spline_space_t::template Derivative<order>;
    using Point = typename spline_space_t::Point;
    using Value = typename spline_space_t::Value;
    using Div   = typename spline_space_t::Div;

    using RefPoint = typename spline_space_t::RefPoint;

public:
    /** Type for the reference face space.*/
    using RefFaceSpace = Conditional<(dim>0),
          NURBSSpace<dim-1, range, rank>,
          NURBSSpace<0, range, rank> >;

    using FaceSpace = PhysicalSpace<RefFaceSpace, typename PushForwardType::FacePushForward>;

    /** Type for the element accessor. */
    using ElementAccessor = NURBSElementAccessor<dim, range, rank> ;

    /** Type for iterator over the elements.  */
    using ElementIterator = GridForwardIterator<ElementAccessor>;

public:
//    /** Container indexed by the components of the space */
    template< class T>
    using ComponentContainer = typename spline_space_t::template ComponentContainer<T>;

    using DegreeTable = typename spline_space_t::DegreeTable;
    using MultiplicityTable = typename spline_space_t::MultiplicityTable;

    using EndBehaviour = typename spline_space_t::EndBehaviour;
    using EndBehaviourTable = typename spline_space_t::EndBehaviourTable;
    using InteriorReg= typename spline_space_t::InteriorReg;
    using SpaceDimensionTable = typename spline_space_t::SpaceDimensionTable;

    using Weights = DynamicMultiArray<Real,dim>;
    using WeightsTable = ComponentContainer<Weights>;



public:
    /** @name Creators*/
    ///@{
    /**
     * Returns a shared_ptr wrapping a maximum regularity NURBSSpace over CartesianGrid
     * @p knots for the given @p degree in all directions and homogeneous in all components.
     * @note All weights are set to 1.0, so the resulting space has the same structure of a BSpline space.
     */
    static std::shared_ptr< self_t >
    create(const int degree,
           std::shared_ptr< GridType > knots,
           const WeightsTable &weights);

    /**
     * Returns a shared_ptr wrapping a maximum regularity NURBSSpace over CartesianGrid
     * @p knots for the given @p degree for each direction and for each component.
     * @note All weights are set to 1.0, so the resulting space has the same structure of a BSpline space.
     */
    static std::shared_ptr< self_t >
    create(const DegreeTable &degree,
           std::shared_ptr<GridType> knots,
           const WeightsTable &weights);

    /**
     * Returns a shared_ptr wrapping a NURBSSpace over the CartesianGrid @p knots with
     * the given multiplicity vector @p mult_vector for each component
     * and for the given @p degree for each direction and for each component.
     */
    static std::shared_ptr<self_t>
    create(const DegreeTable &deg,
           std::shared_ptr<GridType> knots,
           std::shared_ptr<const MultiplicityTable> interior_mult,
           const EndBehaviourTable &ends = EndBehaviourTable(),
           const WeightsTable &weights = WeightsTable());

    /**
     * Returns a shared_ptr wrapping a NURBSSpace from a BSplineSpace and a table of weights.
     */
    static std::shared_ptr<self_t>
    create(std::shared_ptr<spline_space_t> bs_space,
           const WeightsTable &weights);

    ///@}

    /** Destructor */
    ~NURBSSpace() = default;


    std::shared_ptr<SpaceManager> get_space_manager();

    std::shared_ptr<const SpaceManager> get_space_manager() const;


protected:
    /** @name Constructor */
    ///@{

    /**
     * Constructs a maximum regularity NURBSSpace over CartesianGrid
     * @p knots for the given @p degree in all directions and homogeneous in all components.
     * @note All weights are set to 1.0, so the resulting space has the same structure of a BSpline space.
     */
    explicit NURBSSpace(const int degree,
                        std::shared_ptr< GridType > knots,
                        const WeightsTable &weights);

    /**
     * Constructs a maximum regularity NURBSSpace over CartesianGrid
     * @p knots for the given @p degree for each direction and for each component.
     * @note All weights are set to 1.0, so the resulting space has the same structure of a BSpline space.
     */
    explicit NURBSSpace(const DegreeTable &degree,
                        std::shared_ptr<GridType> knots,
                        const WeightsTable &weights);

    /**
     * Construct a NURBSSpace over the CartesianGrid @p knots with
     * the given multiplicity vector @p mult_vector for each component
     * and for the given @p degree for each direction and for each component.
     */
    explicit  NURBSSpace(const DegreeTable &deg,
                         std::shared_ptr<GridType> knots,
                         std::shared_ptr<const MultiplicityTable> interior_mult,
                         const EndBehaviourTable &ends,
                         const WeightsTable &weights);

    /**
     * Construct a NURBSSpace from a BSplineSpace and a table of weights.
     */
    explicit  NURBSSpace(std::shared_ptr<spline_space_t> bs_space,
                         const WeightsTable &weights);

    /**
     * Copy constructor. Not allowed to be used.
     */
    NURBSSpace(const self_t &space) = delete;

    ///@}

public:
    /** @name Getting information about the space */
    ///@{
    /**
     * Total number of dofs (i.e number of basis functions aka dimensionality)
     * of the space.
     */
    Size get_num_basis() const;

    /** Return the total number of dofs for the i-th space component. */
    Size get_num_basis(const int i) const;

    /**
     *  Return the total number of dofs for the i-th space component
     *  and the j-th direction.
     */
    Size get_num_basis(const int comp, const int dir) const;

    /**
     * Component-direction indexed table with the number of basis functions
     * in each direction and component
     */
    const SpaceDimensionTable &get_num_basis_table() const;

    SpaceDimensionTable get_num_all_element_basis() const
    {
    	return sp_space_->get_num_all_element_basis();
    }

    /**
     * Returns the degree of the BSpline space for each component and for each coordinate direction.
     * \return The degree of the BSpline space for each component and for each coordinate direction.
     * The first index of the returned object is the component id, the second index is the direction id.
     */
    const DegreeTable &get_degree() const;

    /**
     * Returns the multiplicity of the internal knots that defines the BSpline
     * space for each component and for each coordinate direction.
     * \return The multiplicity of the internal knots that defines the BSpline
     * space for each component and for each coordinate direction.
     */
    std::shared_ptr<const MultiplicityTable> get_interior_mult() const;


    vector<Index> get_loc_to_global(const CartesianGridElement<dim> &element) const;

    vector<Index> get_loc_to_patch(const CartesianGridElement<dim> &element) const;
    ///@}



    const std::shared_ptr<spline_space_t> get_spline_space() const;

    /** Returns the container with the global dof distribution (const version). */
    const DofDistribution<dim, range, rank> &get_dof_distribution_global() const;

    /** Returns the container with the global dof distribution (non const version). */
    DofDistribution<dim, range, rank> &get_dof_distribution_global();

    /** Returns the container with the patch dof distribution (const version). */
    const DofDistribution<dim, range, rank> &get_dof_distribution_patch() const;

    /** Returns the container with the patch dof distribution (non const version). */
    DofDistribution<dim, range, rank> &get_dof_distribution_patch();



    /** Return the push forward (non-const version). */
    std::shared_ptr<PushForwardType> get_push_forward();

    /** Return the push forward (const version). */
    std::shared_ptr<const PushForwardType> get_push_forward() const;

    std::shared_ptr<const self_t >
    get_reference_space() const;


    std::shared_ptr<RefFaceSpace>
    get_ref_face_space(const Index face_id,
                       vector<Index> &face_to_element_dofs,
                       typename GridType::FaceGridMap &elem_map) const;

    std::shared_ptr<FaceSpace>
    get_face_space(const Index face_id,
                   vector<Index> &face_to_element_dofs) const;



    /**
     * Adds an @p offset to the values of the dof ids.
     */
    void add_dofs_offset(const Index offset);

    /**
    * Returns a element iterator to the first element of the patch
    */
    ElementIterator begin() const;

    /**
     * Returns a element iterator to the last element of the patch
     */
    ElementIterator last() const;

    /**
     * Returns a element iterator to one-pass the end of patch.
     */
    ElementIterator end() const;

    /**
     * Get the weights of the NURBSSpace.
     */
    const WeightsTable &get_weights() const;

    /**
     * Reset the weights of the NURBSSpace.
     */
    void reset_weights(const WeightsTable &weights);

    /**
     * Prints internal information about the space.
     * @note Mostly used for debugging and testing.
     */
    void print_info(LogStream &out) const;


private:
    /**
     * B-spline space
     */
    std::shared_ptr<spline_space_t> sp_space_;

    /**
     * Weights associated to the basis functions.
     */
    WeightsTable weights_;

    /**
     * Refines the NURBSSpace after the uniform refinement of the BSplineSpace.
     *
     * @param[in] refinement_directions Directions along which the refinement is performed.
     * @param[in] grid_old Grid before the refinement.
     *
     * @pre Before invoking this function, must be invoked the function BSplineSpace::refine().
     * @note This function is connected to the CartesianGrid's signal for the refinement, and
     * it is necessary in order to avoid infinite loops in the CartesianGrid::refine() function calls.
     * @note The implementation of this function is based on "The NURBS Book" Algorithm A5.4.
     *
     * @ingroup h_refinement
     */
    void refine_h_weights(const std::array<bool,dim> &refinement_directions,
                          const GridType &grid_old);

    /**
     * Create a signal and a connection for the refinement.
     */
    void create_refinement_connection();


    friend ElementAccessor;

    /**
     * Performs checks after the construction of the object.
     * In debug mode, if something is going wrong, an assertion will be raised.
     *
     * @warning This function should be used as last line in the implementation of each constructor.
     */
    void perform_post_construction_checks() const;
};



IGA_NAMESPACE_CLOSE


#endif /* __NURBS_SPACE_H_ */



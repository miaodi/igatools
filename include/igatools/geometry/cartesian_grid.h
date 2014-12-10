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

#ifndef CARTESIAN_GRID_H_
#define CARTESIAN_GRID_H_

#include <igatools/base/config.h>
#include <igatools/base/tensor.h>
#include <igatools/base/logstream.h>
#include <igatools/utils/cartesian_product_array.h>
#include <igatools/utils/dynamic_multi_array.h>
#include <igatools/geometry/unit_element.h>
#include <igatools/base/array_utils.h>
#include <igatools/geometry/grid_forward_iterator.h>

// TODO (pauletti, Oct 9, 2014): should we use iga array
#include <array>
#include <memory>

#include <boost/signals2.hpp>

IGA_NAMESPACE_OPEN

template <int> class CartesianGridElement;
template <int> class GridElementHandler;
/**
 * @brief Grid in <tt>dim</tt>-dimensional space with cartesian-product structure.
 *
 * Vector of knot vectors without repetition interpreted as a tensor product
 * cartesian grid.
 *
 * ### Getting a CartesianGrid by an XML structure.
 * A CartesianGrid object can be obtained by a Boost XML structure using the
 * function
 * get_cartesian_grid_from_xml().
 * An example of valid XML structure for a 2-dimensional CartesianGrid is given
 * by the following
 * XML block
 * @code{.xml}
   <CartesianGrid Dim="2">
     <Knots Direction="0" Size="2">
       0.0 1.0
     </Knots>
     <Knots Direction="1" Size="2">
        0.0 1.0
     </Knots>
   </CartesianGrid>
 * @endcode
 * where the <tt>Dim</tt> attribute in the <tt>"CartesianGrid"</tt> tag is the
 * dimensionality of the CartesianGrid
 * and the <tt>Size</tt> attribute in each <tt>"Knots"</tt> tag is the number
 *  of unique knots values (also called
 * "breakpoints") along each coordinate direction.
 *
 * @author M. Martinelli 2012, 2013, 2014
 * @author pauletti 2012, 2013, 2014
 *
 * @see get_cartesian_grid_from_xml()
 * @ingroup h_refinement
 * @ingroup containers
 * @todo document more
 */
template<int dim_>
class CartesianGrid :
    protected TensorSizedContainer<dim_>,
    public std::enable_shared_from_this<CartesianGrid<dim_>>
{
private:
    /** Type for current class. */
    using self_t = CartesianGrid<dim_>;

public:
    /** Dimensionality of the grid. */
    static constexpr int dim = dim_;

    static constexpr std::array<Size, dim_> dims = sequence<dim>();

    using Point = Points<dim>;

    /** Type for the element accessor. */
    using ElementAccessor = CartesianGridElement<dim>;

    /** Type for iterator over the elements.  */
    using ElementIterator = CartesianGridIterator<ElementAccessor>;

    using ElementHandler = GridElementHandler<dim>;
    /** Type for the vector of knot vectors */
    using KnotCoordinates = CartesianProductArray<Real, dim>;

    /**
     * Types of grid for future optimization
     */
    enum class Kind
    {
        uniform, direction_uniform, non_uniform
    };

    /** @name Constructors*/
    ///@{
protected:
    /**
     * Construct a uniform cartesian grid of the unit <tt>dim</tt>-dimensional
     * hypercube \f$[0,1]^{dim}\f$, with @p n knots (equally spaced) in each dimension.
     */
    explicit CartesianGrid(const Size n = 2);

    /**
     * Construct a uniform cartesian grid of the unit <tt>dim</tt>-dimensional
     * hypercube \f$[0,1]^{dim}\f$,
     * with <tt>n[0],..,n[dim-1</tt>] knots in each dimension
     * respectively.
     */
    explicit CartesianGrid(const TensorSize<dim> &n_knots, const Kind kind);

    /**
     * @todo Document me
     */
    explicit CartesianGrid(const BBox<dim> &end_points,
                           const Size n_knots,
                           const Kind kind);

    /**
     * @todo Document me
     */
    explicit CartesianGrid(const BBox<dim> &end_points,
                           const TensorSize<dim> &n_knots,
                           const Kind kind);

    /**
     * Construct a cartesian grid where the knot coordinate in each
     * direction is provided as CartesianProductArray object.
     *
     * The knot coordinate in each direction must be sorted and without
     * repetition.
     * @note In Debug mode, a check for this precondition (up to machine precision)
     * is perform and if not satistified an exception is raised.
     */
    explicit
    CartesianGrid(const KnotCoordinates &knot_coordinates,
                  const Kind kind);

    /**
     * Construct a cartesian grid where the knot coordinate in each
     * direction is provided as std::array of vector<Real>.
     *
     * The knot coordinate in each direction must be sorted and without
     * repetition.
     * @note In Debug mode, a check for this precondition (up to machine precision)
     * is perform and if not satistified an exception is raised.
     */
    explicit
    CartesianGrid(const std::array<vector<Real>,dim> &knot_coordinates);

public:
    /**
     * Copy constructor.
     * Perform a deep copy of the member variables except the
     * signal_refine_ variable, that is not copied at all.
     */
    CartesianGrid(const self_t &grid);

    /**  Move constructor */
    CartesianGrid(self_t &&grid) = default;

    /** Destructor */
    ~CartesianGrid() = default;
    ///@}

    /**
     * @name Creators
     * @note The functions here return a CartesianGrid<dim> object wrapped by
     * a std::shared_ptr
     */
    ///@{

    /**
     * Creates a uniform cartesian grid of the unit <tt>dim</tt>-dimensional
     * hypercube \f$[0,1]^{dim}\f$, with @p n knots (equally spaced) in each dimension.
     */
    static std::shared_ptr<self_t> create(const Index n = 2);


    /**
     * Creates a uniform cartesian grid of the unit <tt>dim</tt>-dimensional
     * hypercube \f$[0,1]^{dim}\f$,
     * with <tt>n[0],..,n[dim-1</tt>] knots in each dimension
     * respectively.
     */
    static std::shared_ptr<self_t> create(const TensorSize<dim> &n);


    /**
     * Construct a cartesian grid where the knot coordinate in each
     * direction is provided as CartesianProductArray object.
     *
     * The knot coordinate in each direction must be sorted and without
     * repetition.
     * @note In Debug mode, a check for this precondition
     * (up to machine precision)
     * is perform and if not satisfied an exception is raised.
     */
    static std::shared_ptr<self_t>
    create(const KnotCoordinates &knot_coordinates);

    /**
     * Construct a cartesian grid where the knot coordinate in each
     * direction is provided as std::array of vector<Real>.
     *
     * The knot coordinate in each direction must be sorted and without
     * repetition.
     * @note In Debug mode, a check for this precondition
     * (up to machine precision)
     * is perform and if not satisfied an exception is raised.
     */
    static std::shared_ptr<self_t>
    create(const std::array<vector<Real>,dim> &knot_coordinates);


    static std::shared_ptr<self_t>
    create(const BBox<dim> &end_points, const Size n_knots);

    /**
     * @todo document me
     */
    static std::shared_ptr<self_t>
    create(const BBox<dim> &end_points, const TensorSize<dim> &n);

    /**
     * Creates a CastesianGrid object (wrapped by a shared_ptr) using
     * the copy constructor.
     *
     * @note A grid built in this way is totally uncoupled from the grid used as argument
     * of this function. For example, a refinement of a grid does not affect the other gird.
     */
    static std::shared_ptr<self_t>
    create(const self_t &grid);

    ///@}

    /**
     * @name Assignment operators
     */
    ///@{

    /**
     * Copy assignment operator.
     */
    self_t &operator=(const self_t &grid) = default;

    /**
     * Move assignment operator.
     */
    self_t &operator=(self_t &&grid) = default;
    ///@}


    ///@name Getting grid information
    ///@{
    /**
     * Total number of active elements.
     */
    Size get_num_active_elems() const;

    /** Total number of elements, including active and non-active */
    Size get_num_all_elems() const;

    /**
     * Total number of one dimensional intervals along each
     * coordinate direction.
     */
    TensorSize<dim> get_num_intervals() const;

    /**
     * Query the number of knot values along each coordinate direction
     * represented by the CartesianGrid.
     */
    TensorSize<dim> get_num_knots_dim() const;

    /**
     * Returns the knot coordinates along the direction @p i.
     */
    vector<Real> const &get_knot_coordinates(const int i) const;

    /**
     * Returns the knot coordinates along all the directions (const version).
     */
    KnotCoordinates const &get_knot_coordinates() const;

    /**
     * Computes the interval lengths along each direction.
     */
    CartesianProductArray<Real, dim> get_element_lengths() const;

    /**
     * Returns the smallest <tt>dim</tt>-dimensional bounding box enclosing the
     * domain represented by the CartesianGrid object.
     */
    BBox<dim> get_bounding_box() const;
    ///@}


    ///@name Iterating of grid elements
    ///@{
    /**
     * This function returns a element iterator to the first element of the patch.
     */
    ElementIterator begin() const;

    /**
     * This function returns a element iterator to one-pass the end of patch.
     */
    ElementIterator end() const;
    ///@}

    ElementHandler get_element_handler() const;

    /** @name Functions for the index transformations */
    ///@{
    /**
     * Transformation from a tensor-index to a flat-index.
     */
    Index tensor_to_flat(const TensorIndex<dim_> &tensor_index) const;

    /**
     * Transformation from a flat-index to a tensor-index.
     */
    TensorIndex<dim_> flat_to_tensor(const Index flat_index) const;
    ///@}


    ///@name Dealing with boundary information
    ///@{
    /**
     * Get the patch @p face boundary id.
     */
    boundary_id get_boundary_id(const int face) const;

    /**
     * Set the patch @p face to have the boundary @p id.
     */
    void set_boundary_id(const int face, const boundary_id id);


    template<int sub_dim>
    using BoundaryNormal = std::array<Points<dim>, dim-sub_dim>;
    /**
     * Returns the outward pointing
     * unit normal vector space to the element of sub dim k.
     */
    template<int sub_dim>
    BoundaryNormal<sub_dim> get_boundary_normals(const int s_id) const;

    template<int k>
    using InterGridMap = std::map<typename CartesianGrid<k>::ElementIterator, ElementIterator>;

    /**
     * Construct a sub grid of dimension k conforming to
     * the grid sub element sub_elem_id and a map from the elements of
     * the sub_element grid to the corresponding element of the current
     * grid.
     */
    template<int k>
    std::shared_ptr<CartesianGrid<k> >
    get_sub_grid(const int sub_elem_id, InterGridMap<k> &elem_map) const;

    ///@}

    /**
     * Given a vector of points, this function return a map with
     * entries indexed by the grid element each point belongs to
     * containing a list of indices of the points that belong to
     * this element.
     * For ex.
     * @code
     *
     * @endcode
     */
    std::map<ElementIterator, vector<int> >
    find_elements_of_points(const ValueVector<Points<dim>> &points) const;

public:
    /**
     * Prints debug information of the CartesianGrid to a LogStream.
     */
    void print_info(LogStream &out) const;

    /**
     * Comparison operator. Returns true if the knot coordinates of two grid
     * are equal.
     */
    bool operator==(const CartesianGrid<dim> &grid) const;

private:
    /** Type for the refinement signal. */
    using signal_refine_t =
        boost::signals2::signal<
        void (const std::array<bool,dim> &,const CartesianGrid<dim> &)>;

public:

    /** Slot type for the refinement signal. */
    using SignalRefineSlot = typename signal_refine_t::slot_type;

    /** @name Functions for performing grid refinement */
    ///@{
    /**
     * Perform a uniform refinement of the grid along the @p direction_id
     * direction,
     * dividing each interval into @p n_subdivisions intervals.
     * @param[in] direction_id Direction along which the refinement is
     * performed.
     * @param[in] n_subdivisions Number of subdivision in which each interval
     * in the grid
     * along the specified direction is divided. This value must be >= 2.
     */
    void refine_direction(const int direction_id, const Size n_subdivisions);

    /**
     * Refine the cartesian grid and the objects connected to it (if any),
     * e.g. maps, spaces, etc.
     * along the directions specified by the true values in the entries of the
     * array of bools @p refinement_directions,
     * and with a number of subdivisions for each interval
     * (along each direction)
     * specified by @p n_subdivisions.
     *
     * @note If the i-th direction is not active for the refinement
     * (i.e. <tt>refinement_directions[i] == false</tt>),
     *  then the corresponding value <tt>n_subdivisions[i]</tt> will be ignored.
     */
    void refine_directions(
        const std::array<bool,dim> &refinement_directions,
        const std::array<Size,dim> &n_subdivisions);

    /**
     * Refine the cartesian grid and the objects connected to it (if any),
     * e.g. maps, spaces, etc.
     *
     * Each interval in the unrefined grid is uniformly divided
     * in @p n_subdivisions sub-intervals.
     */
    void refine(const Size n_subdivisions = 2);

    /**
     *  Connect a slot (i.e. a function pointer) to the refinement signals
     *  which will be
     *  emitted whenever a refine() function is called by an object holding
     *  a CartesianGrid member.
     */
    boost::signals2::connection
    connect_refinement(const SignalRefineSlot &subscriber);

    /**
     * Returns the grid before the last refinement. If no refinement is
     * performed, this function returns a null pointer.
     */
    std::shared_ptr<const self_t > get_grid_pre_refinement() const;
    ///@}

private:
    /** Flag for optimization use */
    Kind kind_ = Kind::non_uniform;

    /** Boundary ids, one id per face */
    std::array<boundary_id, UnitElement<dim>::template num_elem<dim-1>()> boundary_id_;

    /**
     *  Knot coordinates along each coordinate direction.
     *  For each direction the knot coordinates are sorted in an increasing
     *  order and does not contain duplicate values.
     */
    KnotCoordinates knot_coordinates_;

    /**
     * In the hierarchical spaces elements are characterized as influent or not
     * this is the place where this information is stored.
     */
    DynamicMultiArray<bool,dim> marked_elems_;

    /**
     * Active elements indicators (used for example in hierarchical spaces).
     */
    DynamicMultiArray<bool,dim> active_elems_;

    /**
     * Perform a uniform refinement of the knots along the @p direction_id
     * direction,
     * dividing each interval in the knot vector into @p n_subdivisions
     * intervals.
     * @param[in] direction_id Direction along which the refinement is
     * performed.
     * @param[in] n_subdivisions Number of subdivision in which each interval
     * in the knot vector is
     * divided. This value must be >= 2.
     */
    void refine_knots_direction(const int direction_id,
                                const Size n_subdivisions);

    /**
     * This class member is the grid before the last refinement. If no
     * refinement is performed, this is a null pointer.
     */
    std::shared_ptr<const self_t> grid_pre_refinement_ = nullptr;

    /**
     * Signals for the h-refinement. It can be viewed as a FIFO list of
     * function pointers.
     */
    signal_refine_t refine_signals_;

    friend class CartesianGridElement<dim>;
};

//template<int dim>
//std::shared_ptr<CartesianGrid::SubGrid<dim-1>>
//get_face_grid(const int sub_elem_id, InterGridMap<k> &elem_map) const;

IGA_NAMESPACE_CLOSE

#endif /* CARTESIAN_GRID_H_ */

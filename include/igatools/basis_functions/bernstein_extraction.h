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

#ifndef __BERNSTEIN_EXTRACTION_H_
#define __BERNSTEIN_EXTRACTION_H_

#include <igatools/base/config.h>

#include <igatools/basis_functions/spline_space.h>
#include <igatools/linear_algebra/dense_matrix.h>

IGA_NAMESPACE_OPEN

/**
 *
 * @ingroup serializable
 */
class BernsteinOperator : public DenseMatrix
{
public:
  using Values = DenseMatrix;
  using DenseMatrix::DenseMatrix;

  Values scale_action(const Real scale, const Values &b_values) const;


private:

#ifdef IGATOOLS_WITH_SERIALIZATION
  /**
   * @name Functions needed for serialization
   */
  ///@{
  friend class cereal::access;
  template<class Archive>
  void
  serialize(Archive &ar);
  ///@}
#endif // IGATOOLS_WITH_SERIALIZATION

};

/**
 * @brief Class used to store the coefficients that describe the B-Spline basis function as
 * linear combination of the Bernstein polynomial.
 *
 * A spline function restricted to each interval determined by
 * the knots is a polynomial of order m.
 *
 * In particular each B-spline can be expressed a linear combination
 * of the Bernstein polynomial.
 *
 * This class computes and stores theses coefficients.
 *
 * @ingroup serializable
 */
template<int dim, int range = 1, int rank = 1>
class BernsteinExtraction
{
public:
  using Operator = BernsteinOperator;
  using Space = SplineSpace<dim, range, rank>;
  using DegreeTable = typename Space::DegreeTable;
  using KnotsTable = typename Space::KnotsTable;
  using MultiplicityTable = typename Space::MultiplicityTable;
  using EndBehaviourTable = typename Space::EndBehaviourTable;

  using ElemOper = SafeSTLArray<Operator const *, dim>;
  using ElemOperTable = typename Space::template ComponentContainer<ElemOper>;
private:
  using Operators = SafeSTLArray<SafeSTLVector<Operator>,dim>;
  using OperatorsTable = typename Space::template ComponentContainer<Operators>;

public:
  /**
   * Default constructor. It does nothing but it is needed for the serialization.
   */
  BernsteinExtraction() = default;

  /**
   * Construct the extraction operators.
   */
  BernsteinExtraction(const Grid<dim> &grid,
                      const KnotsTable &rep_knots,
                      const MultiplicityTable &acum_mult,
                      const DegreeTable &deg);


  /**
   * Print the class content
   */
  void print_info(LogStream &out) const;


  const Operator &
  get_operator(const int dir, const int inter, const int comp) const;


  ElemOperTable get_element_operators(TensorIndex<dim> idx) const;



private:
  SafeSTLVector<Operator>
  fill_extraction(const int m,
                  const SafeSTLVector<Real>    &knots,
                  const SafeSTLVector<Real>    &rep_knots,
                  const SafeSTLVector<Index>   &acum_mult);

  /** Given the M_{j-1} computes and returns the M_{j} */
  Operator compute(const Operator &M_j_1,
                   typename SafeSTLVector<Real>::const_iterator  y,
                   const Real a,
                   const Real b);

  OperatorsTable ext_operators_;

#ifdef IGATOOLS_WITH_SERIALIZATION
  /**
   * @name Functions needed for serialization
   */
  ///@{
  friend class cereal::access;
  template<class Archive>
  void serialize(Archive &ar);
  ///@}
#endif // IGATOOLS_WITH_SERIALIZATION
};


IGA_NAMESPACE_CLOSE


#ifdef IGATOOLS_WITH_SERIALIZATION

CEREAL_SPECIALIZE_FOR_ARCHIVE(IArchive,iga::BernsteinOperator,cereal::specialization::member_serialize)
CEREAL_SPECIALIZE_FOR_ARCHIVE(OArchive,iga::BernsteinOperator,cereal::specialization::member_serialize)

//#include <igatools/basis_functions/bernstein_extraction.serial>

#endif // IGATOOLS_WITH_SERIALIZATION

#endif

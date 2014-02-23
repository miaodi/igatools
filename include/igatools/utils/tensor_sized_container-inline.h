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

// QualityAssurance: martinelli, 28 Jan 2014


#ifndef TENSOR_SIZED_CONTAINER_INLINE_H_
#define TENSOR_SIZED_CONTAINER_INLINE_H_


#include <igatools/utils/tensor_sized_container.h>
#include <igatools/utils/multi_array_utils.h>

IGA_NAMESPACE_OPEN


template <int rank>
inline
TensorSize<rank>
TensorSizedContainer<rank>::
tensor_size() const
{
    return size_;
}


template <int rank>
inline
Size
TensorSizedContainer<rank>::
flat_size() const
{
    return size_.flat_size();
}


template <int rank>
inline
void
TensorSizedContainer<rank>::
reset_size(const TensorSize<rank> &size)
{
    size_ = size;
    weight_ = MultiArrayUtils<rank>::compute_weight(size_) ;
}



template<int rank>
inline
Index
TensorSizedContainer<rank>::
tensor_to_flat(const TensorIndex<rank> &tensor_index) const
{
    return MultiArrayUtils<rank>::tensor_to_flat_index(tensor_index, weight_);
}


template<int rank>
inline
TensorIndex<rank>
TensorSizedContainer<rank>::
flat_to_tensor(const Index flat_index) const
{
    return MultiArrayUtils<rank>::flat_to_tensor_index(flat_index, weight_);
}



IGA_NAMESPACE_CLOSE


#endif // #ifndef TENSOR_SIZED_CONTAINER_INLINE_H_

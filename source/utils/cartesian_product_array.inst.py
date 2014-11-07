#-+--------------------------------------------------------------------
# Igatools a general purpose Isogeometric analysis library.
# Copyright (C) 2012-2014  by the igatools authors (see authors.txt).
#
# This file is part of the igatools library.
#
# The igatools library is free software: you can use it, redistribute
# it and/or modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation, either
# version 3 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#-+--------------------------------------------------------------------


# QA (pauletti, Mar 19, 2014):
from init_instantiation_data import *

iga_inc_files = ['base/tensor.h',
				 'utils/value_table.h']
other_files   = ['igatools/linear_algebra/dense_matrix.h' ]
data = Instantiation(iga_inc_files, other_files)
(f, inst) = (data.file_output, data.inst)

types = ('Real','Real*', 'Index')
for dim in inst.domain_dims:
    for t in types:
        row = 'CartesianProductArray<%s,%d>' %(t,dim)
        f.write('template class %s; \n' % (row))
        for k in range(0, dim+1):
            f.write('template %s::SubProduct<%d> ' %(row, k) +
                    '%s::get_sub_product(const TensorIndex<%d> &index) const; \n'  %(row, k))
matrix = 'DenseMatrix'
types = (matrix, "const %s *" %matrix, ) + \
	('vector<%s>' %matrix, 'const vector<%s> *' %matrix)
#types = types + ('Real','Real*', 'Index')
ma_list = ['CartesianProductArray<%s,%d>' %(t,dim) 
           for dim in inst.domain_dims for t in types]

for row in ma_list:
	f.write('template class %s; \n' % (row))


types = ['double','Index']
for dim in inst.domain_dims:
	if dim >= 1:
		for type in types:
			ret_type = 'CartesianProductArray<%s,%d>' %(type,dim)
			arg_type = 'CartesianProductArray<%s,%d>' %(type,dim-1)
			f.write('template %s insert(const %s &,const int,const vector<%s>&); \n' % (ret_type,arg_type,type))


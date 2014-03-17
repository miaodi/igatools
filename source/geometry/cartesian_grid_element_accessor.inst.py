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

# QA (pauletti, Mar 4, 2014 ):
from init_instantiation_data import *
file_output, inst = intialize_instantiation()

inc = '#include <../source/geometry/grid_forward_iterator.cpp>\n'
file_output.write(inc)
file_output.write('IGA_NAMESPACE_OPEN\n')

accessors = ['CartesianGridElementAccessor<%d>' %(dim) for dim in inst.domain_dims]
for row in accessors:
    file_output.write('template class %s; \n' %(row))
    file_output.write('template class %s::ValuesCache<0>; \n' %(row))
    file_output.write('template class %s::ValuesCache<1>; \n' %(row))
    file_output.write('template class GridForwardIterator<%s>;\n' %(row))

file_output.write('IGA_NAMESPACE_CLOSE\n')
file_output.close()





    
    
	

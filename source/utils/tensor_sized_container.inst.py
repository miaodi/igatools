#-+--------------------------------------------------------------------
# Igatools a general purpose Isogeometric analysis library.
# Copyright (C) 2012-2016  by the igatools authors (see authors.txt).
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
data = Instantiation()
(f, inst) = (data.file_output, data.inst)

ts_list=['TensorSizedContainer<%d>' %dim for dim in inst.all_domain_dims]
for row in ts_list:
   f.write('template class %s; \n' % (row))



#---------------------------------------------------
f.write('#ifdef IGATOOLS_WITH_SERIALIZATION\n')

archives = ['OArchive','IArchive']

for ts in unique(ts_list):
    for ar in archives:
        f.write('template void %s::serialize(%s&);\n' %(ts,ar))
        
f.write('#endif // IGATOOLS_WITH_SERIALIZATION\n')
#---------------------------------------------------

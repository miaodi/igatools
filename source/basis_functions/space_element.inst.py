#-+--------------------------------------------------------------------
# Igatools a general purpose Isogeometric analysis library.
# Copyright (C) 2012-2015  by the igatools authors (see authors.txt).
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

from init_instantiation_data import *

include_files = ['geometry/cartesian_grid.h',
                 'geometry/grid_element.h',
                 'basis_functions/space_element.h',
                 '../../source/geometry/grid_iterator.cpp']
data = Instantiation(include_files)
(f, inst) = (data.file_output, data.inst)


sub_dim_members = \
 ['ValueVector<Real> elem::get_w_measures<k>(const int) const;']


elements = ['SpaceElement<0,0,0,1,Transformation::h_grad>']
templated_funcs = ['ValueVector<Real> SpaceElement<0,0,0,1,Transformation::h_grad>::get_w_measures<0>(const int) const;']


transformations = ['Transformation::h_grad']


#--------------------------------------------------------------------------------------
# SpaceElement used by ReferenceSpaceElement 
for x in inst.sub_ref_sp_dims:
    for t in transformations:
        elem = 'SpaceElement<%d,0,%d,%d,%s>' %(x.dim, x.range, x.rank,t)
        elements.append(elem)
        for func in sub_dim_members:
            k = x.dim
            s = func.replace('elem', elem).replace('k', '%d' % (k));
            templated_funcs.append(s)
            
            
for x in inst.ref_sp_dims:
    for t in transformations:
        elem = 'SpaceElement<%d,0,%d,%d,%s>' %(x.dim, x.range, x.rank,t)
        elements.append(elem)
        for func in sub_dim_members:
            for k in inst.sub_dims(x.dim):
                s = func.replace('elem', elem).replace('k', '%d' % (k));
                templated_funcs.append(s)
#--------------------------------------------------------------------------------------


#--------------------------------------------------------------------------------------
# SpaceElement used by PhysicalSpaceElement 
for space in inst.SubPhysSpaces:
    x = space.spec
    for t in transformations:
        elem = 'SpaceElement<%d,%d,%d,%d,%s>' %(x.dim,x.codim,x.range, x.rank,t)
        elements.append(elem)
        for func in sub_dim_members:
            k = x.dim
            s = func.replace('elem', elem).replace('k', '%d' % (k));
            templated_funcs.append(s)


for space in inst.SubPhysSpaces + inst.PhysSpaces:
    x = space.spec
    for t in transformations:
        elem = 'SpaceElement<%d,%d,%d,%d,%s>' %(x.dim,x.codim,x.range, x.rank,t)
        elements.append(elem)
        for func in sub_dim_members:
            for k in inst.sub_dims(x.dim):
                s = func.replace('elem', elem).replace('k', '%d' % (k));
                templated_funcs.append(s)
#--------------------------------------------------------------------------------------



#---------------------------------------------------
for elem in unique(elements):
    f.write('template class %s ;\n' %elem)
    for it in inst.iterators:
        iterator = it.replace('Accessor','%s' % (elem) )
        f.write('template class %s; \n' %iterator)

for func in unique(templated_funcs):
    f.write('template %s ;\n' %func)
#---------------------------------------------------


#---------------------------------------------------
f.write('IGA_NAMESPACE_CLOSE\n')

f.write('#ifdef SERIALIZATION\n')
id = 0 
for elem in unique(elements):
    alias = 'SpaceElementAlias%d' %(id)
    f.write('using %s = iga::%s; \n' % (alias, elem))
    f.write('BOOST_CLASS_EXPORT_IMPLEMENT(%s) \n' %alias)
    f.write('template void %s::serialize(OArchive &, const unsigned int);\n' % alias)
    f.write('template void %s::serialize(IArchive &, const unsigned int);\n' % alias)
    id += 1 
f.write('#endif // SERIALIZATION\n')
    
f.write('IGA_NAMESPACE_OPEN\n')
#---------------------------------------------------



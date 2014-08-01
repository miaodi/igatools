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


#include <igatools/basis_functions/dofs_manager.h>
#include <igatools/base/exceptions.h>

#include <map>
#include <set>

using std::vector;
using std::map;
using std::set;
using std::pair;

IGA_NAMESPACE_OPEN





DofsManager::
DofsManager()
    :
    is_space_insertion_open_(false),
//    dofs_view_(nullptr),
    num_unique_dofs_(0)
{}



void
DofsManager::
space_insertion_open()
{
    is_space_insertion_open_ = true;
}


void
DofsManager::
space_insertion_close(const bool automatic_dofs_renumbering)
{
    Assert(is_space_insertion_open_ == true,ExcInvalidState());

    Assert(!spaces_info_.empty(),ExcEmptyObject());


    //--------------------------------------------------------------------------
    std::vector<DofsComponentView> dofs_components_view;

    Index offset = 0;
    for (auto &space_info : spaces_info_)
    {
        space_info.second.min_dofs_id_ += offset;
        space_info.second.max_dofs_id_ += offset;

        auto &dofs_view = space_info.second.dofs_view_;

        if (automatic_dofs_renumbering)
        {
            for (Index &dof : dofs_view)
                dof += offset;

            offset += space_info.second.n_dofs_;
        }

        auto view_ranges = dofs_view.begin().get_ranges();
        dofs_components_view.insert(dofs_components_view.end(),view_ranges.begin(),view_ranges.end());
    }
    //--------------------------------------------------------------------------

    DofsIterator dofs_begin(dofs_components_view,0);;
    DofsIterator dofs_end(dofs_components_view,IteratorState::pass_the_end);

    dofs_view_ = DofsView(dofs_begin,dofs_end);

    is_space_insertion_open_ = false;


    num_unique_dofs_ = this->count_unique_dofs();
}


DofsManager::
SpaceInfo::
SpaceInfo(const SpacePtrVariant &space,
          const Index n_dofs,
          const Index min_dofs_id,
          const Index max_dofs_id,
          const DofsView &dofs_view)
    :
    space_(space),
    min_dofs_id_(min_dofs_id),
    max_dofs_id_(max_dofs_id),
    dofs_view_(dofs_view)
{
    Assert(n_dofs > 0,ExcEmptyObject());
}



auto
DofsManager::
get_dofs_view() -> DofsView &
{
    Assert(is_space_insertion_open_ == false,ExcInvalidState());

//    Assert(dofs_view_ != nullptr, ExcNullPtr())
    return dofs_view_;
}


auto
DofsManager::
get_dofs_view() const -> DofsConstView
{
    Assert(is_space_insertion_open_ == false,ExcInvalidState());

//    Assert(dofs_view_ != nullptr, ExcNullPtr())
    return DofsConstView(dofs_view_);
}



Index
DofsManager::
get_num_dofs() const
{
    return num_unique_dofs_;
}



Index
DofsManager::
get_num_linear_constraints() const
{
    return linear_constraints_.size();
}


Index
DofsManager::
get_num_equality_constraints() const
{
    return equality_constraints_.size();
}


Index
DofsManager::
get_global_dof(const int space_id, const Index local_dof) const
{
    Assert(is_space_insertion_open_ == false,ExcInvalidState());

    Assert(space_id >= 0,ExcLowerRange(space_id,0));

    return spaces_info_.at(space_id).dofs_view_[local_dof];
}


std::vector<Index>
DofsManager::
get_global_dofs(const int space_id, const std::vector<Index> &local_dofs) const
{
    Assert(!local_dofs.empty(),ExcEmptyObject());

    std::vector<Index> global_dofs;

    for (const Index local_dof : local_dofs)
        global_dofs.emplace_back(this->get_global_dof(space_id,local_dof));

    return global_dofs;
}



bool
DofsManager::
is_space_insertion_open() const
{
    return is_space_insertion_open_;
}


bool
DofsManager::
are_elements_dofs_view_open() const
{
    return are_elements_dofs_view_open_;
}


auto
DofsManager::
get_elements_dofs_view() const -> const std::vector<DofsConstView> &
{
    return elements_dofs_view_;
}


void
DofsManager::
elements_dofs_view_open()
{
    Assert(are_elements_dofs_view_open_ == false,
           ExcMessage("Element dofs view already opened."));
    are_elements_dofs_view_open_ = true;
}


void
DofsManager::
elements_dofs_view_close()
{
    Assert(are_elements_dofs_view_open_ == true,
           ExcMessage("Element dofs view already closed."));
    are_elements_dofs_view_open_ = false;
}


void
DofsManager::
add_element_dofs_view(const DofsConstView &element_dofs_view)
{
//    Assert(is_space_insertion_open_ == false,ExcInvalidState());

    Assert(are_elements_dofs_view_open_ == true,ExcInvalidState());

    elements_dofs_view_.push_back(element_dofs_view);
}



void
DofsManager::
equality_constraints_open()
{
    Assert(are_equality_constraints_open_ == false,
           ExcMessage("Equality constraints already opened."));
    are_equality_constraints_open_ = true;
}


void
DofsManager::
equality_constraints_close()
{
    Assert(are_equality_constraints_open_ == true,
           ExcMessage("Equality constraints already closed."));
    are_equality_constraints_open_ = false;

//    Assert(false,ExcNotImplemented());
//    AssertThrow(false,ExcNotImplemented());
}



void
DofsManager::
add_equality_constraint(const Index dof_id_master,const Index dof_id_slave)
{
    Assert(are_equality_constraints_open_ == true,
           ExcMessage("Equality constraints already closed."));

    equality_constraints_.emplace_back(EqualityConstraint(dof_id_master,dof_id_slave));
}




void
DofsManager::
linear_constraints_open()
{
    Assert(are_linear_constraints_open_ == false,
           ExcMessage("Linear constraints already opened."));
    are_linear_constraints_open_ = true;

    Assert(false,ExcNotImplemented());
    AssertThrow(false,ExcNotImplemented());
}


void
DofsManager::
linear_constraints_close()
{
    Assert(are_linear_constraints_open_ == true,
           ExcMessage("Linear constraints already closed."));
    are_linear_constraints_open_ = false;

    Assert(false,ExcNotImplemented());
    AssertThrow(false,ExcNotImplemented());
}



void
DofsManager::
remove_equality_constraints_redundancies()
{
    map<Index,set<Index>> upper_sparsity_pattern_pre;

    for (const auto &eq_constr : equality_constraints_)
    {
        // retrieving the set of slaves dofs corresponding to the master dof
        auto &set_slaves_id = upper_sparsity_pattern_pre[eq_constr.get_dof_id_master()];

        // inserting the slave dof
        set_slaves_id.insert(eq_constr.get_dof_id_slave());
    }
    equality_constraints_.clear();

    map<Index,set<Index>> upper_sparsity_pattern_post;

    for (const auto &row_m : upper_sparsity_pattern_pre)
    {
        const Index master_id = row_m.first;
        for (const auto &slave_id : row_m.second)
        {
            auto &set_slaves_id_post = upper_sparsity_pattern_post[master_id];
            set_slaves_id_post.insert(slave_id);
            if (upper_sparsity_pattern_pre.count(slave_id) > 0)
            {
                auto &row_s = upper_sparsity_pattern_pre[slave_id];

                for (const auto &new_slave_id: row_s)
                    set_slaves_id_post.insert(new_slave_id);

                row_s.clear();
            }
        }
    }

    for (const auto &row_m : upper_sparsity_pattern_post)
    {
        const Index master_id = row_m.first;
        for (const auto &slave_id : row_m.second)
            equality_constraints_.emplace_back(EqualityConstraint(master_id,slave_id));
    }
}




Index
DofsManager::
count_unique_dofs() const
{
    Assert(is_space_insertion_open_ == false,ExcInvalidState());

//    const auto &dofs = this->get_dofs_view();

    set<Index> unique_dofs(dofs_view_.begin(),dofs_view_.end());

    return unique_dofs.size();
}






void
DofsManager::
print_info(LogStream &out) const
{
    using std::endl;

    std::string tab("    ");

    out << "DofsManager infos:" << endl;

    out.push(tab);


    Assert(is_space_insertion_open_ == false,ExcInvalidState());
    Assert(are_equality_constraints_open_ == false,ExcInvalidState());
    Assert(are_linear_constraints_open_ == false,ExcInvalidState());

//    Assert(dofs_view_ != nullptr, ExcNullPtr())
    out << "DOFs = [ ";
    for (const Index &dof : dofs_view_)
        out << dof << " ";
    out << "]" << endl;


    Assert(!spaces_info_.empty(),ExcEmptyObject());
    Index i = 0;
    for (auto &space_info : spaces_info_)
    {

        out << "Space["<< i <<"]:   ID=" << space_info.first
            << "   n_dofs=" << space_info.second.n_dofs_
            << "   DOFs=[ ";

        DofsView &dofs_space_view = const_cast<DofsView &>(space_info.second.dofs_view_);
        for (Index &dof : dofs_space_view)
            out << dof << " ";
        out << "]" << endl;

        i++;
        //*/
    }

    out << "Num. unique dofs          = " << this->get_num_dofs() << endl;
    out << "Num. linear   constraints = " << this->get_num_linear_constraints() << endl;



    out << "Num. equality constraints = " << this->get_num_equality_constraints() << endl;
    out.push(tab);
    i = 0;
    for (const auto &eq_constr : equality_constraints_)
    {
        out << "Eq. constraint[" << i++ << "]: ";
        eq_constr.print_info(out);
        out << endl;
    }
    out.pop();


    out.pop();
}


IGA_NAMESPACE_CLOSE

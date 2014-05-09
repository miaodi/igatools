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


#include <igatools/linear_algebra/linear_solver.h>


#ifdef USE_TRILINOS
#include <BelosTpetraAdapter.hpp>
#include <BelosSolverFactory.hpp>


#ifdef REAL_IS_LONG_DOUBLE
namespace Teuchos
{
template<>
iga::Real
ScalarTraits<iga::Real>::zero()
{
    return iga::Real(0.0);
}

template<>
iga::Real
ScalarTraits<iga::Real>::one()
{
    return iga::Real(1.0);
}

template<>
iga::Real
ScalarTraits<iga::Real>::eps()
{
    return std::numeric_limits<iga::Real>::epsilon();
}

template<>
string
ScalarTraits<iga::Real>::name()
{
    return "iga::Real";
}

template<>
iga::Real
ScalarTraits<iga::Real>::rmax()
{
    return std::numeric_limits<iga::Real>::max();
}

template<>
auto
ScalarTraits<iga::Real>::Real(iga::Real number) -> magnitudeType
{
    return std::Real(number);
}

template<>
auto
ScalarTraits<iga::Real>::conjugate(iga::Real number) -> magnitudeType
{
    return std::Real(number);
}

template<>
auto
ScalarTraits<iga::Real>::magnitude(iga::Real number) -> magnitudeType
{
    return std::abs(number);
}

template<>
auto
ScalarTraits<iga::Real>::squareroot(iga::Real number) -> magnitudeType
{
    return std::sqrt(number);
}


template<>
iga::Real
EnhancedNumberTraits<iga::Real>::defaultStep()
{
    return iga::Real(1.0);
}

template<>
unsigned short
EnhancedNumberTraits<iga::Real>::defaultPrecision()
{
    return 100;
}

};
#endif // #ifdef REAL_IS_LONG_DOUBLE

using namespace Teuchos ;

#endif // #ifdef USE_TRILINOS




IGA_NAMESPACE_OPEN


#ifdef USE_TRILINOS

LinearSolver<LinearAlgebraPackage::trilinos>::
LinearSolver(const SolverType solver_type, const Real tolerance, const int max_num_iter)
    :
    solver_params_(parameterList())
{
    // map the SolverType enum elements to the name aliases used by Belos
    solver_type_enum_to_alias_[to_integral(SolverType::GMRES)] = "GMRES";
    solver_type_enum_to_alias_[to_integral(SolverType::FlexibleGMRES)] = "Flexible GMRES";
    solver_type_enum_to_alias_[to_integral(SolverType::CG)] = "CG";
    solver_type_enum_to_alias_[to_integral(SolverType::StochasticCG)] = "Stochastic CG";
    solver_type_enum_to_alias_[to_integral(SolverType::RecyclingCG)] = "Recycling CG";
    solver_type_enum_to_alias_[to_integral(SolverType::RecyclingGMRES)] = "Recycling GMRES";
    solver_type_enum_to_alias_[to_integral(SolverType::PseudoBlockGMRES)] = "Pseudo Block GMRES";
    solver_type_enum_to_alias_[to_integral(SolverType::PseudoBlockCG)] = "Pseudo Block CG";


    const std::string solver_name = solver_type_enum_to_alias_[to_integral(solver_type)] ;

    Belos::SolverFactory<Real,vector_t,matrix_t> factory;
    //
    // "Num Blocks" = Maximum number of Krylov vectors to store.  This
    // is also the restart length.  "Block" here refers to the ability
    // of this particular solver (and many other Belos solvers) to solve
    // multiple linear systems at a time, even though we are only solving
    // one linear system in this example.
    solver_params_->set("Num Blocks", 40);
    solver_params_->set("Maximum Iterations", max_num_iter);
    solver_params_->set("Convergence Tolerance", tolerance);

    // Create the solver.
    solver_ = factory.create(solver_name,solver_params_);
}


void
LinearSolver<LinearAlgebraPackage::trilinos>::
set_solver_parameters(Teuchos::RCP<Teuchos::ParameterList> solver_params)
{
    solver_params_ = solver_params;
    solver_->setParameters(solver_params_);
}

void
LinearSolver<LinearAlgebraPackage::trilinos>::
set_max_num_iterations(const int max_num_iter)
{
    solver_params_->set("Maximum Iterations", max_num_iter);
    solver_->setParameters(solver_params_);
}

/**
 * Set the level that residual norms must reach to decide convergence.
 */
void
LinearSolver<LinearAlgebraPackage::trilinos>::
set_tolerance(const Real tolerance)
{
    solver_params_->set("Convergence Tolerance", tolerance);
    solver_->setParameters(solver_params_);
}


void
LinearSolver<LinearAlgebraPackage::trilinos>::
solve(Matrix<LinearAlgebraPackage::trilinos> &A,
      Vector<LinearAlgebraPackage::trilinos> &b,
      Vector<LinearAlgebraPackage::trilinos> &x)
{
    // Create a LinearProblem struct with the problem to solve.
    // A, X, B, and M are passed by (smart) pointer, not copied.
    auto linear_system = rcp(
                             new Belos::LinearProblem<Real,vector_t,matrix_t>(
                                 A.get_trilinos_matrix(),
                                 x.get_trilinos_vector(),
                                 b.get_trilinos_vector()));
    linear_system->setProblem() ;

    solver_->setProblem(linear_system);

    // Attempt to solve the linear system.  result == Belos::Converged
    // means that it was solved to the desired tolerance.  This call
    // overwrites X with the computed approximate solution.
    Belos::ReturnType result = solver_->solve();

    AssertThrow(result == Belos::ReturnType::Converged, ExcMessage("No convergence."));
}


Real
LinearSolver<LinearAlgebraPackage::trilinos>::
get_achieved_tolerance() const
{
    return solver_->achievedTol() ;
}

int
LinearSolver<LinearAlgebraPackage::trilinos>::
get_num_iterations() const
{
    return solver_->getNumIters() ;
}

#endif // #ifdef USE_TRILINOS







#ifdef USE_PETSC

LinearSolver<LinearAlgebraPackage::petsc>::
LinearSolver(const SolverType solver_type, const Real tolerance, const int max_num_iter)
{
    PetscErrorCode ierr;
    comm_ = PETSC_COMM_WORLD;
    std::string prec_name;

    // map the SolverType enum elements to the name aliases used by PETSc
    solver_type_enum_to_alias_[to_integral(SolverType::GMRES)] = "gmres";
    solver_type_enum_to_alias_[to_integral(SolverType::CG)] = "cg";
    solver_type_enum_to_alias_[to_integral(SolverType::LU)] = "preonly";

    const std::string solver_name = solver_type_enum_to_alias_[to_integral(solver_type)] ;


    if (solver_type == SolverType::LU)
        prec_name = "lu" ;
    else
        prec_name = "none" ;

    ierr = KSPCreate(comm_, &ksp_);
    ierr = KSPGetPC(ksp_,&pc_);

    ierr = PCSetType(pc_,prec_name.c_str());
    ierr = KSPSetType(ksp_,solver_name.c_str());

    ierr = KSPSetTolerances(ksp_,tolerance,PETSC_DEFAULT,PETSC_DEFAULT,max_num_iter);
}

LinearSolver<LinearAlgebraPackage::petsc>::
LinearSolver(const SolverType solver_type, const PreconditionerType prec_type,
             const Real tolerance, const int max_num_iter)
{
    PetscErrorCode ierr;
    comm_ = PETSC_COMM_WORLD;

    // map the SolverType enum elements to the name aliases used by PETSc
    solver_type_enum_to_alias_[to_integral(SolverType::GMRES)] = "gmres";
    solver_type_enum_to_alias_[to_integral(SolverType::CG)] = "cg";
    solver_type_enum_to_alias_[to_integral(SolverType::LU)] = "preonly";

    // map the PreconditionerType enum elements to the name aliases used by PETSc
    prec_type_enum_to_alias_[to_integral(PreconditionerType::NONE)] = "none";
    prec_type_enum_to_alias_[to_integral(PreconditionerType::ILU)] = "ilu";
    prec_type_enum_to_alias_[to_integral(PreconditionerType::JACOBI)] = "jacobi";

    const std::string solver_name = solver_type_enum_to_alias_[to_integral(solver_type)] ;
    std::string prec_name = prec_type_enum_to_alias_[to_integral(prec_type)] ;

    if (solver_type == SolverType::LU)
        prec_name = "lu" ;

    ierr = KSPCreate(comm_, &ksp_);
    ierr = KSPGetPC(ksp_,&pc_);

    ierr = PCSetType(pc_,prec_name.c_str());
    ierr = KSPSetType(ksp_,solver_name.c_str());

    ierr = KSPSetTolerances(ksp_,tolerance,PETSC_DEFAULT,PETSC_DEFAULT,max_num_iter);
}

/*
void
LinearSolver<LinearAlgebraPackage::petsc>::
set_solver_parameters(Teuchos::RCP<Teuchos::ParameterList> solver_params)
{
    solver_params_ = solver_params;
    solver_->setParameters(solver_params_);
}
//*/

void
LinearSolver<LinearAlgebraPackage::petsc>::
set_max_num_iterations(const int max_num_iter)
{
    PetscErrorCode ierr;
    PetscReal rtol, abstol, dtol;
    PetscInt maxits;
    ierr = KSPGetTolerances(ksp_, &rtol, &abstol, &dtol, &maxits);
    ierr = KSPSetTolerances(ksp_, rtol, abstol, dtol, max_num_iter);

}

/**
 * Set the level that residual norms must reach to decide convergence.
 */
void
LinearSolver<LinearAlgebraPackage::petsc>::
set_tolerance(const Real tolerance)
{
    PetscErrorCode ierr;
    PetscReal rtol, abstol, dtol;
    PetscInt maxits;
    ierr = KSPGetTolerances(ksp_, &rtol, &abstol, &dtol, &maxits);
    ierr = KSPSetTolerances(ksp_, tolerance, abstol, dtol, maxits);
}


void
LinearSolver<LinearAlgebraPackage::petsc>::
solve(Matrix<LinearAlgebraPackage::petsc> &A,
      Vector<LinearAlgebraPackage::petsc> &b,
      Vector<LinearAlgebraPackage::petsc> &x)
{
    PetscErrorCode ierr;
    ierr = KSPSetOperators(ksp_,A.get_petsc_matrix(),A.get_petsc_matrix(),SAME_NONZERO_PATTERN);
    ierr = KSPSolve(ksp_,b.get_petsc_vector(),x.get_petsc_vector());
}


Real
LinearSolver<LinearAlgebraPackage::petsc>::
get_achieved_tolerance() const
{
    PetscErrorCode ierr;
    PetscReal achieved_tol;
    ierr = KSPGetResidualNorm(ksp_, &achieved_tol);

    return achieved_tol;
}

int
LinearSolver<LinearAlgebraPackage::petsc>::
get_num_iterations() const
{
    PetscErrorCode ierr;
    int num_iters;
    ierr = KSPGetIterationNumber(ksp_, &num_iters);

    return num_iters;
}

#endif // #ifdef USE_PETSC


IGA_NAMESPACE_CLOSE



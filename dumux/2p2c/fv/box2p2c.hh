#ifndef DUNE_BOX2P2C_HH
#define DUNE_BOX2P2C_HH

#include<dune/istl/operators.hh>
#include<dune/istl/solvers.hh>
#include<dune/istl/preconditioners.hh>

//#include <dune/common/array.hh>        // defines simple array class
#include <dune/common/fixedarray.hh>   // defines simple array classes
#include <dune/common/geometrytype.hh>
#include <dune/grid/sgrid.hh>          // a complete structured grid
#include <dune/grid/common/referenceelements.hh>
#include <dune/grid/common/gridinfo.hh>
#include <dune/grid/common/universalmapper.hh>
#include <dune/grid/common/quadraturerules.hh>
#include <dune/common/collectivecommunication.hh>
#include <dune/istl/io.hh>
#include <dune/common/timer.hh>
#include <dune/istl/bvector.hh>
#include <dune/istl/vbvector.hh>
#include <dune/istl/bcrsmatrix.hh>
#include <dune/istl/io.hh>
#include <dune/istl/gsetc.hh>
#include <dune/istl/ilu.hh>
#include <dune/istl/operators.hh>
#include <dune/istl/solvers.hh>
#include <dune/istl/preconditioners.hh>
#include <dune/istl/scalarproducts.hh>
#include <dune/istl/paamg/amg.hh>
#include <dune/grid/common/scsgmapper.hh>
#include <dune/grid/common/mcmgmapper.hh>
#include <dune/disc/functions/functions.hh>
#include <dune/disc/functions/p0function.hh>
#include <dune/disc/functions/p1function.hh>
#include <dune/disc/operators/p1operator.hh>
#include <dune/disc/operators/boundaryconditions.hh>
#include <dune/disc/groundwater/groundwater.hh>
#include <dune/disc/groundwater/p1groundwater.hh>
#include <dune/disc/groundwater/p1groundwaterestimator.hh>
#include <dune/grid/io/file/vtk/vtkwriter.hh>
#include <dune/istl/paamg/amg.hh>
#include "dumux/nonlinear/newtonmethod.hh"
#include "dumux/twophase/twophasemodel.hh"
#include "dumux/twophase/twophaseproblem.hh"
#include "dumux/2p2c/fv/box2p2cjacobian.hh"

namespace Dune
{
  template<class G, class RT>
  class Box2P2C 
  : public LeafP1TwoPhaseModel<G, RT, TwoPhaseProblem<G, RT>, 
                                 Box2P2CJacobian<G, RT> >
  {
  public:
	// define the problem type (also change the template argument above)
	typedef TwoPhaseProblem<G, RT> ProblemType;
	
	// define the local Jacobian (also change the template argument above)
	typedef Box2P2CJacobian<G, RT> LocalJacobian;
	
	typedef LeafP1TwoPhaseModel<G, RT, ProblemType, LocalJacobian> LeafP1TwoPhaseModel;

	typedef Box2P2C<G, RT> ThisType;
	
	Box2P2C(const G& g, ProblemType& prob) 
	: LeafP1TwoPhaseModel(g, prob)
	{ 	}

	void solve() 
	{
		typedef typename LeafP1TwoPhaseModel::FunctionType::RepresentationType VectorType;
		typedef typename LeafP1TwoPhaseModel::OperatorAssembler::RepresentationType MatrixType;
		typedef MatrixAdapter<MatrixType,VectorType,VectorType> Operator; 
		
		Operator op(*(this->A));  // make operator out of matrix
		double red=1E-8;
		SeqILU0<MatrixType,VectorType,VectorType> ilu0(*(this->A),1.0);// a precondtioner
		BiCGSTABSolver<VectorType> solver(op,ilu0,red,100,1);         // an inverse operator 
		InverseOperatorResult r;
		solver.apply(*(this->u), *(this->f), r);
		
		return;		
	}

	void update (double& dt)
	{
		this->localJacobian.setDt(dt);
		this->localJacobian.setOldSolution(this->uOldTimeStep);
		NewtonMethod<G, ThisType> newtonMethod(this->grid, *this);
		newtonMethod.execute();
		*(this->uOldTimeStep) = *(this->u);
		
		return;
	}

  };

}
#endif

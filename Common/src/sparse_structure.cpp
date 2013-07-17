/*!
 * \file sparse_structure.cpp
 * \brief Main subroutines for doing the sparse structures.
 * \author Aerospace Design Laboratory (Stanford University) <http://su2.stanford.edu>.
 * \version 2.0.5
 *
 * Stanford University Unstructured (SU2) Code
 * Copyright (C) 2012 Aerospace Design Laboratory
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../include/sparse_structure.hpp"

CSparseMatrix::CSparseMatrix(void) {

  /*--- Array initialization ---*/
	matrix            = NULL;
	row_ptr           = NULL;
	col_ind           = NULL;
	block             = NULL;
	prod_block_vector = NULL;
	prod_row_vector   = NULL;
	aux_vector        = NULL;
  invM              = NULL;
  LineletBool       = NULL;
  LineletPoint      = NULL;

}

CSparseMatrix::~CSparseMatrix(void) {
  
	if (matrix != NULL) delete [] matrix;
	if (row_ptr != NULL) delete [] row_ptr;
	if (col_ind != NULL) delete [] col_ind;
	if (block != NULL) delete [] block;
	if (prod_block_vector != NULL) delete [] prod_block_vector;
	if (prod_row_vector != NULL) delete [] prod_row_vector;
	if (aux_vector != NULL) delete [] aux_vector;
  if (invM != NULL) delete [] invM;
  if (LineletBool != NULL) delete [] LineletBool;
  if (LineletPoint != NULL) delete [] LineletPoint;

}

void CSparseMatrix::SetIndexes(unsigned long val_nPoint, unsigned long val_nPointDomain, unsigned short val_nVar, unsigned short val_nEq, unsigned long* val_row_ptr, unsigned long* val_col_ind, unsigned long val_nnz, bool preconditioner) {
    
	nPoint = val_nPoint;                // Assign number of points in the mesh
	nPointDomain = val_nPointDomain;	// Assign number of points in the mesh
	nVar = val_nVar;                    // Assign number of vars in each block system
	nEqn = val_nEq;                    // Assign number of eqns in each block system
	nnz = val_nnz;                      // Assign number of possible non zero blocks
	row_ptr = val_row_ptr;
	col_ind = val_col_ind;
	
	matrix = new double [nnz*nVar*nEqn];	// Reserve memory for the values of the matrix
	block = new double [nVar*nEqn];
	prod_block_vector = new double [nEqn]; //correct?
	prod_row_vector = new double [nVar]; //correct?
	aux_vector = new double [nVar]; //correct?
	
	if (preconditioner)
		invM = new double [nPoint*nVar*nEqn];	// Reserve memory for the values of the inverse of the preconditioner
        
}

void CSparseMatrix::GetBlock(unsigned long block_i, unsigned long block_j) {
	unsigned long step = 0, index, iVar;
	
	for (index = row_ptr[block_i]; index < row_ptr[block_i+1]; index++) {
		step++;
		if (col_ind[index] == block_j) {
			for (iVar = 0; iVar < nVar*nEqn; iVar++)
				block[iVar] = matrix[(row_ptr[block_i]+step-1)*nVar*nEqn+iVar];
			break;
		}
	}
}

void CSparseMatrix::DisplayBlock(void) {
	unsigned short iVar, jVar;
	
	for (iVar = 0; iVar < nVar; iVar++) {
		for (jVar = 0; jVar < nEqn; jVar++)
			cout << block[iVar*nEqn+jVar] << "  ";
		cout << endl;
	}
}

void CSparseMatrix::ReturnBlock(double **val_block) {
	unsigned short iVar, jVar;
	for (iVar = 0; iVar < nVar; iVar++)
		for (jVar = 0; jVar < nEqn; jVar++)
			val_block[iVar][jVar] = block[iVar*nEqn+jVar];
}

void CSparseMatrix::AddBlock(unsigned long block_i, unsigned long block_j, double **val_block) {
	unsigned long iVar, jVar, index, step = 0;
	
	for (index = row_ptr[block_i]; index < row_ptr[block_i+1]; index++) {
		step++;
		if (col_ind[index] == block_j) {
			for (iVar = 0; iVar < nVar; iVar++)
				for (jVar = 0; jVar < nEqn; jVar++)
					matrix[(row_ptr[block_i]+step-1)*nVar*nEqn+iVar*nEqn+jVar] += val_block[iVar][jVar];
			break;
		}
	}
}

void CSparseMatrix::SubtractBlock(unsigned long block_i, unsigned long block_j, double **val_block) {
	unsigned long iVar, jVar, index, step = 0;
	
	for (index = row_ptr[block_i]; index < row_ptr[block_i+1]; index++) {
		step++;
		if (col_ind[index] == block_j) {
			for (iVar = 0; iVar < nVar; iVar++)
				for (jVar = 0; jVar < nEqn; jVar++)
					matrix[(row_ptr[block_i]+step-1)*nVar*nEqn+iVar*nEqn+jVar] -= val_block[iVar][jVar];
			break;
		}
	}
}

void CSparseMatrix::AddVal2Diag(unsigned long block_i, double val_matrix) {
	unsigned long step = 0, iVar, index;
	
	for (index = row_ptr[block_i]; index < row_ptr[block_i+1]; index++) {
		step++;
		if (col_ind[index] == block_i) {	// Only elements on the diagonal
			for (iVar = 0; iVar < nVar; iVar++)
				matrix[(row_ptr[block_i]+step-1)*nVar*nVar+iVar*nVar+iVar] += val_matrix;
			break;
		}
	}
}

void CSparseMatrix::AddVal2Diag(unsigned long block_i,  double* val_matrix, unsigned short num_dim) {
	unsigned long step = 0, iVar, iSpecies;
	
	for (unsigned long index = row_ptr[block_i]; index < row_ptr[block_i+1]; index++) {
		step++;
		if (col_ind[index] == block_i) {	// Only elements on the diagonal
			for (iVar = 0; iVar < nVar; iVar++) {
				iSpecies = iVar/(num_dim + 2);
				matrix[(row_ptr[block_i]+step-1)*nVar*nVar+iVar*nVar+iVar] += val_matrix[iSpecies];
			}
			break;
		}
	}
}

void CSparseMatrix::AddVal2Diag(unsigned long block_i,  double* val_matrix, unsigned short val_nDim, unsigned short val_nDiatomics) {
	unsigned long step = 0, iVar, iSpecies;
	
	for (unsigned long index = row_ptr[block_i]; index < row_ptr[block_i+1]; index++) {
		step++;
		if (col_ind[index] == block_i) {	// Only elements on the diagonal
			for (iVar = 0; iVar < nVar; iVar++) {
        if (iVar < (val_nDim+3)*val_nDiatomics) iSpecies = iVar / (val_nDim+3);
        else iSpecies = (iVar - (val_nDim+3)*val_nDiatomics) / (val_nDim+2) + val_nDiatomics;
				matrix[(row_ptr[block_i]+step-1)*nVar*nVar+iVar*nVar+iVar] += val_matrix[iSpecies];
			}
			break;
		}
	}
}


void CSparseMatrix::DeleteValsRowi(unsigned long i) {
	unsigned long block_i = i/nVar;
	unsigned long row = i - block_i*nVar;
	unsigned long index, iVar;

	for (index = row_ptr[block_i]; index < row_ptr[block_i+1]; index++) {
		for (iVar = 0; iVar < nVar; iVar++)
			matrix[index*nVar*nVar+row*nVar+iVar] = 0.0; // Delete row values in the block
		if (col_ind[index] == block_i)
			matrix[index*nVar*nVar+row*nVar+row] = 1.0; // Set 1 to the diagonal element
	}
}

double CSparseMatrix::SumAbsRowi(unsigned long i) {
	unsigned long block_i = i/nVar;
	unsigned long row = i - block_i*nVar;

	double sum = 0;
	for (unsigned long index = row_ptr[block_i]; index < row_ptr[block_i+1]; index++)
		for (unsigned long iVar = 0; iVar < nVar; iVar ++)
			sum += fabs(matrix[index*nVar*nVar+row*nVar+iVar]);

	return sum;
}

void CSparseMatrix::Gauss_Elimination(unsigned long block_i, double* rhs) {
	unsigned short jVar, kVar;
	short iVar;
	double weight;
    
	GetBlock(block_i,block_i);
    
	if (nVar == 1)
		rhs[0] /= (block[0]+EPS*EPS);
	else {
        
        /*--- Transform system in Upper Matrix ---*/
        for (iVar = 1; iVar < (short)nVar; iVar++) {
            for (jVar = 0; jVar < iVar; jVar++) {
                weight = block[iVar*nVar+jVar]/(block[jVar*nVar+jVar]+EPS*EPS);
                for (kVar = jVar; kVar < nVar; kVar++)
                    block[iVar*nVar+kVar] -= weight*block[jVar*nVar+kVar];
                rhs[iVar] -= weight*rhs[jVar];
            }
        }
        
        /*--- Backwards substitution ---*/
        double aux;
        rhs[nVar-1] = rhs[nVar-1]/(block[nVar*nVar-1]+EPS*EPS);
        for (iVar = nVar-2; iVar >= 0; iVar--) {
            aux = 0;
            for (jVar = iVar+1; jVar < nVar; jVar++)
                aux += block[iVar*nVar+jVar]*rhs[jVar];
            rhs[iVar] = (rhs[iVar]-aux)/(block[iVar*nVar+iVar]+EPS*EPS);
            if (iVar == 0) break;
        }
	}
}

void CSparseMatrix::Gauss_Elimination(double* Block, double* rhs) {
	unsigned short jVar, kVar;
	short iVar;
	double weight;
	
	/*--- Copy block matrix, note that the original matrix 
	 is modified by the algorithm---*/
	for (kVar = 0; kVar < nVar; kVar++)
		for (jVar = 0; jVar < nVar; jVar++)
			block[kVar*nVar+jVar] = Block[kVar*nVar+jVar];

	if (nVar == 1)
		rhs[0] /= (block[0]+EPS*EPS);
	else {			
		/*--- Transform system in Upper Matrix ---*/
		for (iVar = 1; iVar < (short)nVar; iVar++) {
			for (jVar = 0; jVar < iVar; jVar++) {
				weight = block[iVar*nVar+jVar]/(block[jVar*nVar+jVar]+EPS*EPS);
				for (kVar = jVar; kVar < nVar; kVar++)
					block[iVar*nVar+kVar] -= weight*block[jVar*nVar+kVar];
				rhs[iVar] -= weight*rhs[jVar];
			}
		}
		
		/*--- Backwards substitution ---*/
		double aux;
		rhs[nVar-1] = rhs[nVar-1]/(block[nVar*nVar-1]+EPS*EPS);
		for (iVar = nVar-2; iVar >= 0; iVar--) {
			aux = 0;
			for (jVar = iVar+1; jVar < nVar; jVar++)
				aux += block[iVar*nVar+jVar]*rhs[jVar];
			rhs[iVar] = (rhs[iVar]-aux)/(block[iVar*nVar+iVar]+EPS*EPS);
			if (iVar == 0) break;
		}
	}
	
}

void CSparseMatrix::ProdBlockVector(unsigned long block_i, unsigned long block_j, double* vec) {
	unsigned long j = block_j*nVar;
	unsigned short iVar, jVar;

	GetBlock(block_i,block_j);

	for (iVar = 0; iVar < nVar; iVar++) {
		prod_block_vector[iVar] = 0;
		for (jVar = 0; jVar < nVar; jVar++)
			prod_block_vector[iVar] += block[iVar*nVar+jVar]*vec[j+jVar];
	}
}

void CSparseMatrix::UpperProduct(double* vec, unsigned long row_i) {
	unsigned long iVar, index;

	for (iVar = 0; iVar < nVar; iVar++)
		prod_row_vector[iVar] = 0;

	for (index = row_ptr[row_i]; index < row_ptr[row_i+1]; index++) {
		if (col_ind[index] > row_i) {
			ProdBlockVector(row_i, col_ind[index], vec);
			for (iVar = 0; iVar < nVar; iVar++)
				prod_row_vector[iVar] += prod_block_vector[iVar];
		}
	}
}

void CSparseMatrix::LowerProduct(double* vec, unsigned long row_i) {
	unsigned long iVar, index;

	for (iVar = 0; iVar < nVar; iVar++)
		prod_row_vector[iVar] = 0;

	for (index = row_ptr[row_i]; index < row_ptr[row_i+1]; index++) {
		if (col_ind[index] < row_i) {
			ProdBlockVector(row_i, col_ind[index], vec);
			for (iVar = 0; iVar < nVar; iVar++)
				prod_row_vector[iVar] += prod_block_vector[iVar];
		}
	}
  
}

void CSparseMatrix::DiagonalProduct(double* vec, unsigned long row_i) {
	unsigned long iVar, index;

	for (iVar = 0; iVar < nVar; iVar++)
		prod_row_vector[iVar] = 0;

	for (index = row_ptr[row_i]; index < row_ptr[row_i+1]; index++) {
		if (col_ind[index] == row_i) {
			ProdBlockVector(row_i,col_ind[index],vec);
			for (iVar = 0; iVar < nVar; iVar++)
				prod_row_vector[iVar] += prod_block_vector[iVar];
		}
	}
}

void CSparseMatrix::SendReceive_Solution(double* x, CGeometry *geometry, CConfig *config) {
  unsigned short iVar, iMarker, MarkerS, MarkerR;
	unsigned long iVertex, iPoint, nVertexS, nVertexR, nBufferS_Vector, nBufferR_Vector;
	double *Buffer_Receive = NULL, *Buffer_Send = NULL;
	int send_to, receive_from;
  
#ifndef NO_MPI
  MPI::Status status;
  MPI::Request send_request, recv_request;
#endif
  
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) {
    
		if ((config->GetMarker_All_Boundary(iMarker) == SEND_RECEIVE) &&
        (config->GetMarker_All_SendRecv(iMarker) > 0)) {
			
			MarkerS = iMarker;  MarkerR = iMarker+1;
      
      send_to = config->GetMarker_All_SendRecv(MarkerS)-1;
			receive_from = abs(config->GetMarker_All_SendRecv(MarkerR))-1;
			
			nVertexS = geometry->nVertex[MarkerS];  nVertexR = geometry->nVertex[MarkerR];
			nBufferS_Vector = nVertexS*nVar;        nBufferR_Vector = nVertexR*nVar;
      
      /*--- Allocate Receive and send buffers  ---*/
      Buffer_Receive = new double [nBufferR_Vector];
      Buffer_Send = new double[nBufferS_Vector];
      
      /*--- Copy the solution that should be sended ---*/
      for (iVertex = 0; iVertex < nVertexS; iVertex++) {
        iPoint = geometry->vertex[MarkerS][iVertex]->GetNode();
        for (iVar = 0; iVar < nVar; iVar++)
          Buffer_Send[iVertex*nVar+iVar] = x[iPoint*nVar+iVar];
      }
      
#ifndef NO_MPI
      
//      /*--- Send/Receive using non-blocking communications ---*/
//      send_request = MPI::COMM_WORLD.Isend(Buffer_Send, nBufferS_Vector, MPI::DOUBLE, 0, send_to);
//      recv_request = MPI::COMM_WORLD.Irecv(Buffer_Receive, nBufferR_Vector, MPI::DOUBLE, 0, receive_from);
//      send_request.Wait(status);
//      recv_request.Wait(status);
      
      /*--- Send/Receive information using Sendrecv ---*/
      MPI::COMM_WORLD.Sendrecv(Buffer_Send, nBufferS_Vector, MPI::DOUBLE, send_to, 0,
                               Buffer_Receive, nBufferR_Vector, MPI::DOUBLE, receive_from, 0);
      
#else
      
      /*--- Receive information without MPI ---*/
      for (iVertex = 0; iVertex < nVertexR; iVertex++) {
        iPoint = geometry->vertex[MarkerR][iVertex]->GetNode();
        for (iVar = 0; iVar < nVar; iVar++)
          Buffer_Receive[iVar*nVertexR+iVertex] = Buffer_Send[iVar*nVertexR+iVertex];
      }
      
#endif
      
      /*--- Deallocate send buffer ---*/
      delete [] Buffer_Send;
      
      /*--- Do the coordinate transformation ---*/
      for (iVertex = 0; iVertex < nVertexR; iVertex++) {
        
        /*--- Find point and its type of transformation ---*/
        iPoint = geometry->vertex[MarkerR][iVertex]->GetNode();
        
        /*--- Copy transformed conserved variables back into buffer. ---*/
        for (iVar = 0; iVar < nVar; iVar++)
          x[iPoint*nVar+iVar] = Buffer_Receive[iVertex*nVar+iVar];
        
      }
      
      /*--- Deallocate receive buffer ---*/
      delete [] Buffer_Receive;
      
    }
    
	}
  
}

void CSparseMatrix::RowProduct(double* vec, unsigned long row_i) {
	unsigned long iVar, index;

	for (iVar = 0; iVar < nVar; iVar++)
		prod_row_vector[iVar] = 0;

	for (index = row_ptr[row_i]; index < row_ptr[row_i+1]; index++) {
		ProdBlockVector(row_i,col_ind[index],vec);
		for (iVar = 0; iVar < nVar; iVar++)
			prod_row_vector[iVar] += prod_block_vector[iVar];
	}
}

void CSparseMatrix::MatrixVectorProduct(double* vec, double* prod) {
	unsigned long iPoint, iVar;

	for (iPoint = 0; iPoint < nPointDomain; iPoint++) {
		RowProduct(vec, iPoint);
		for (iVar = 0; iVar < nVar; iVar++)
			prod[iPoint*nVar+iVar] = prod_row_vector[iVar];
	}
}

void CSparseMatrix::MatrixVectorProduct(const CSysVector & vec, CSysVector & prod, CGeometry *geometry, CConfig *config) {
	unsigned long prod_begin, vec_begin, mat_begin, index, iVar, jVar, row_i, iVertex, iPoint, nVertexS,
  nVertexR, nBufferS_Vector, nBufferR_Vector;
  unsigned short iMarker, MarkerS, MarkerR;
	double *Buffer_Receive = NULL, *Buffer_Send = NULL;
	int send_to, receive_from;
  
#ifndef NO_MPI
  MPI::Status status;
  MPI::Request send_request, recv_request;
#endif
  
	/*--- Some checks for consistency between CSparseMatrix and the CSysVectors ---*/
	if ( (nVar != vec.GetNVar()) || (nVar != prod.GetNVar()) ) {
		cerr << "CSparseMatrix::MatrixVectorProduct(const CSysVector&, CSysVector): "
    << "nVar values incompatible." << endl;
		throw(-1);
	}
	if ( (nPoint != vec.GetNBlk()) || (nPoint != prod.GetNBlk()) ) {
		cerr << "CSparseMatrix::MatrixVectorProduct(const CSysVector&, CSysVector): "
    << "nPoint and nBlk values incompatible." << endl;
		throw(-1);
	}
  
	prod = 0.0; // set all entries of prod to zero
	for (row_i = 0; row_i < nPointDomain; row_i++) {
		prod_begin = row_i*nVar; // offset to beginning of block row_i
		for (index = row_ptr[row_i]; index < row_ptr[row_i+1]; index++) {
			vec_begin = col_ind[index]*nVar; // offset to beginning of block col_ind[index]
			mat_begin = (index*nVar*nVar); // offset to beginning of matrix block[row_i][col_ind[indx]]
			for (iVar = 0; iVar < nVar; iVar++) {
				for (jVar = 0; jVar < nVar; jVar++) {
					prod[(const unsigned int)(prod_begin+iVar)] += matrix[(const unsigned int)(mat_begin+iVar*nVar+jVar)]*vec[(const unsigned int)(vec_begin+jVar)];
				}
			}
		}
	}
  
  /*--- MPI Parallelization ---*/
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) {
    
		if ((config->GetMarker_All_Boundary(iMarker) == SEND_RECEIVE) &&
        (config->GetMarker_All_SendRecv(iMarker) > 0)) {
			
			MarkerS = iMarker;  MarkerR = iMarker+1;
      
      send_to = config->GetMarker_All_SendRecv(MarkerS)-1;
			receive_from = abs(config->GetMarker_All_SendRecv(MarkerR))-1;
			
			nVertexS = geometry->nVertex[MarkerS];  nVertexR = geometry->nVertex[MarkerR];
			nBufferS_Vector = nVertexS*nVar;        nBufferR_Vector = nVertexR*nVar;
      
      /*--- Allocate Receive and send buffers  ---*/
      Buffer_Receive = new double [nBufferR_Vector];
      Buffer_Send = new double[nBufferS_Vector];
      
      /*--- Copy the solution that should be sended ---*/
      for (iVertex = 0; iVertex < nVertexS; iVertex++) {
        iPoint = geometry->vertex[MarkerS][iVertex]->GetNode();
        for (iVar = 0; iVar < nVar; iVar++)
          Buffer_Send[iVertex*nVar+iVar] = prod[(const unsigned int)(iPoint*nVar+iVar)];
      }
      
#ifndef NO_MPI
      
//      /*--- Send/Receive using non-blocking communications ---*/
//      send_request = MPI::COMM_WORLD.Isend(Buffer_Send, nBufferS_Vector, MPI::DOUBLE, 0, send_to);
//      recv_request = MPI::COMM_WORLD.Irecv(Buffer_Receive, nBufferR_Vector, MPI::DOUBLE, 0, receive_from);
//      send_request.Wait(status);
//      recv_request.Wait(status);
      
      /*--- Send/Receive information using Sendrecv ---*/
      MPI::COMM_WORLD.Sendrecv(Buffer_Send, nBufferS_Vector, MPI::DOUBLE, send_to, 0,
                               Buffer_Receive, nBufferR_Vector, MPI::DOUBLE, receive_from, 0);
      
#else
      
      /*--- Receive information without MPI ---*/
      for (iVertex = 0; iVertex < nVertexR; iVertex++) {
        iPoint = geometry->vertex[MarkerR][iVertex]->GetNode();
        for (iVar = 0; iVar < nVar; iVar++)
          Buffer_Receive[iVar*nVertexR+iVertex] = Buffer_Send[iVar*nVertexR+iVertex];
      }
      
#endif
      
      /*--- Deallocate send buffer ---*/
      delete [] Buffer_Send;
      
      /*--- Do the coordinate transformation ---*/
      for (iVertex = 0; iVertex < nVertexR; iVertex++) {
        
        /*--- Find point and its type of transformation ---*/
        iPoint = geometry->vertex[MarkerR][iVertex]->GetNode();
        
        /*--- Copy transformed conserved variables back into buffer. ---*/
        for (iVar = 0; iVar < nVar; iVar++) {
          prod[(const unsigned int)(iPoint*nVar+iVar)] = Buffer_Receive[iVertex*nVar+iVar];
        }
        
      }
      
      /*--- Deallocate receive buffer ---*/
      delete [] Buffer_Receive;
      
    }
    
	}
  
}

void CSparseMatrix::GetMultBlockBlock(double *c, double *a, double *b) {
	unsigned long iVar, jVar, kVar;
	
	for(iVar = 0; iVar < nVar; iVar++)
		for(jVar = 0; jVar < nVar; jVar++) {
			c[iVar*nVar+jVar] = 0.0;
			for(kVar = 0; kVar < nVar; kVar++)
				c[iVar*nVar+jVar] += a[iVar*nVar+kVar] * b[kVar*nVar+jVar];
		}
}

void CSparseMatrix::GetMultBlockVector(double *c, double *a, double *b) {
	unsigned long iVar, jVar;
	
	for(iVar = 0; iVar < nVar; iVar++) {
		c[iVar] =  0.0;
		for(jVar = 0; jVar < nVar; jVar++)
			c[iVar] += a[iVar*nVar+jVar] * b[jVar];
	}
}

void CSparseMatrix::GetSubsBlock(double *c, double *a, double *b) {
	unsigned long iVar, jVar;
	
	for(iVar = 0; iVar < nVar; iVar++) 
		for(jVar = 0; jVar < nVar; jVar++)
			c[iVar*nVar+jVar] = a[iVar*nVar+jVar] - b[iVar*nVar+jVar];
}

void CSparseMatrix::GetSubsVector(double *c, double *a, double *b) {
	unsigned long iVar;
	
	for(iVar = 0; iVar < nVar; iVar++) 
		c[iVar] = a[iVar] - b[iVar];
}

void CSparseMatrix::InverseBlock(double *Block, double *invBlock) {
	unsigned long iVar, jVar;
		
	for (iVar = 0; iVar < nVar; iVar++) {
		for (jVar = 0; jVar < nVar; jVar++)
			aux_vector[jVar] = 0.0;
		aux_vector[iVar] = 1.0;
		
		/*--- Compute the i-th column of the inverse matrix ---*/
		Gauss_Elimination(Block, aux_vector);
		
		for (jVar = 0; jVar < nVar; jVar++)
			invBlock[jVar*nVar+iVar] = aux_vector[jVar];			
	}
	
}

void CSparseMatrix::InverseDiagonalBlock(unsigned long block_i, double **invBlock) {

	unsigned long iVar, jVar;

	for (iVar = 0; iVar < nVar; iVar++) {
		for (jVar = 0; jVar < nVar; jVar++)
			aux_vector[jVar] = 0.0;
		aux_vector[iVar] = 1.0;

		/*--- Compute the i-th column of the inverse matrix ---*/
		Gauss_Elimination(block_i, aux_vector);
		for (jVar = 0; jVar < nVar; jVar++)
			invBlock[jVar][iVar] = aux_vector[jVar];
	}

}

void CSparseMatrix::BuildJacobiPreconditioner(void) {
	unsigned long iPoint, iVar, jVar;
	double **invBlock;

	/*--- Small nVar x nVar matrix for intermediate computations ---*/
	invBlock = new double* [nVar];
	for (iVar = 0; iVar < nVar; iVar++)
		invBlock[iVar] = new double [nVar];

	/*--- Compute Jacobi Preconditioner ---*/
	for (iPoint = 0; iPoint < nPoint; iPoint++) {

		/*--- Compute the inverse of the diagonal block ---*/
		InverseDiagonalBlock(iPoint, invBlock);

		/*--- Set the inverse of the matrix to the invM structure (which is a vector) ---*/
		for (iVar = 0; iVar < nVar; iVar++)
			for (jVar = 0; jVar < nVar; jVar++)
				invM[iPoint*nVar*nVar+iVar*nVar+jVar] = invBlock[iVar][jVar];
	}

	for (iVar = 0; iVar < nVar; iVar++)
		delete [] invBlock[iVar];
	delete [] invBlock;
}

void CSparseMatrix::BuildLineletPreconditioner(CGeometry *geometry, CConfig *config) {
	
	/*--- Identify the linelets of the grid ---*/
	bool *check_Point, add_point;
	unsigned long iEdge, iPoint, jPoint, index_Point, iLinelet, iVertex, next_Point, counter, iElem;
	unsigned short iMarker, iNode, ExtraLines = 100;
	double alpha = 0.9, weight, max_weight, *normal, area, volume_iPoint, volume_jPoint, MeanPoints;
	
	check_Point = new bool [geometry->GetnPoint()]; 
	for (iPoint = 0; iPoint < geometry->GetnPoint(); iPoint++) 
		check_Point[iPoint] = true;
	
	/*--- Memory allocation --*/
	nLinelet = 0;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) {
		if ((config->GetMarker_All_Boundary(iMarker) == HEAT_FLUX)  ||
        (config->GetMarker_All_Boundary(iMarker) == ISOTHERMAL) ||
        (config->GetMarker_All_Boundary(iMarker) == EULER_WALL) ||
        (config->GetMarker_All_Boundary(iMarker) == DISPLACEMENT_BOUNDARY)) {
			nLinelet += geometry->nVertex[iMarker];
		}
	}
	
	if (nLinelet == 0) {
		cout << " No linelet structure. Jacobi preconditioner will be used" << endl;
        //if (config->GetContainerPosition(RunTime_EqSystem) != ADJTURB_SOL)
            config->SetKind_Linear_Solver_Prec(JACOBI);
        //else
        //    config->SetKind_AdjTurb_Linear_Prec(JACOBI);
	}
	else {
		
		/*--- Basic initial allocation ---*/
		LineletPoint = new vector<unsigned long>[nLinelet + ExtraLines];
		
		/*--- Define the basic linelets, starting from each vertex ---*/
		for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) {
      if ((config->GetMarker_All_Boundary(iMarker) == HEAT_FLUX)  ||
          (config->GetMarker_All_Boundary(iMarker) == ISOTHERMAL) ||
          (config->GetMarker_All_Boundary(iMarker) == EULER_WALL) ||
          (config->GetMarker_All_Boundary(iMarker) == DISPLACEMENT_BOUNDARY)){
				iLinelet = 0;
				for (iVertex = 0; iVertex < geometry->nVertex[iMarker]; iVertex++) {
					iPoint = geometry->vertex[iMarker][iVertex]->GetNode();
					LineletPoint[iLinelet].push_back(iPoint);
					check_Point[iPoint] = false;
					iLinelet++;
				}
			}
		}
		
		/*--- Create the linelet structure ---*/
		iLinelet = 0;
		do {
			add_point = true;
			index_Point = 0;
			do {
        
				/*--- Compute the value of the max weight ---*/
				iPoint = LineletPoint[iLinelet][index_Point];
				max_weight = 0.0;
				for(iNode = 0; iNode < geometry->node[iPoint]->GetnPoint(); iNode++) {
					jPoint = geometry->node[iPoint]->GetPoint(iNode);
					if ((check_Point[jPoint]) && geometry->node[jPoint]->GetDomain()){
						iEdge = geometry->FindEdge(iPoint, jPoint);
						normal = geometry->edge[iEdge]->GetNormal();
						if (geometry->GetnDim() == 3) area = sqrt(normal[0]*normal[0]+normal[1]*normal[1]+normal[2]*normal[2]);
						else area = sqrt(normal[0]*normal[0]+normal[1]*normal[1]);
						volume_iPoint = geometry->node[iPoint]->GetVolume();
						volume_jPoint = geometry->node[jPoint]->GetVolume();		
						weight = 0.5*area*((1.0/volume_iPoint)+(1.0/volume_jPoint));
						max_weight = max(max_weight, weight);
					}
				}
				
				/*--- Verify if any face of the control volume must be added ---*/
				add_point = false;
				counter = 0;
				for(iNode = 0; iNode < geometry->node[iPoint]->GetnPoint(); iNode++) {
					jPoint = geometry->node[iPoint]->GetPoint(iNode);
					iEdge = geometry->FindEdge(iPoint, jPoint);
					normal = geometry->edge[iEdge]->GetNormal();
					if (geometry->GetnDim() == 3) area = sqrt(normal[0]*normal[0]+normal[1]*normal[1]+normal[2]*normal[2]);
					else area = sqrt(normal[0]*normal[0]+normal[1]*normal[1]);
					volume_iPoint = geometry->node[iPoint]->GetVolume();
					volume_jPoint = geometry->node[jPoint]->GetVolume();		
					weight = 0.5*area*((1.0/volume_iPoint)+(1.0/volume_jPoint));
					if (((check_Point[jPoint]) && (weight/max_weight > alpha) && (geometry->node[jPoint]->GetDomain())) && 
							((index_Point == 0) || ((index_Point > 0) && (jPoint != LineletPoint[iLinelet][index_Point-1])))) {
						add_point = true;
						next_Point = jPoint;
						counter++;
					}
				}
				
				/*--- We have arrived to a isotropic zone, the normal linelet is stopped ---*/
				if (counter > 1) add_point = false;			
				
				/*--- Add a typical point to the linelet ---*/
				if (add_point) {
					LineletPoint[iLinelet].push_back(next_Point);
					check_Point[next_Point] = false;
					index_Point++;
				}
				
				/*--- Leading edge... it is necessary to create two linelets ---*/
				if ((counter == 2) && (index_Point == 0)) {
					LineletPoint[iLinelet].push_back(next_Point);
					check_Point[next_Point] = false;
					index_Point++;
					add_point = true;
					LineletPoint[nLinelet].push_back(LineletPoint[iLinelet][0]);
					nLinelet++;
				}
				
			} while (add_point);
			iLinelet++;
		} while (iLinelet < nLinelet);
		
		delete [] check_Point; 
		
		MeanPoints = 0.0;
		for (iLinelet = 0; iLinelet < nLinelet; iLinelet++)
			MeanPoints += double (LineletPoint[iLinelet].size());
    MeanPoints /= double(nLinelet);
    
		cout << "Points in the linelet structure: " << MeanPoints << "." << endl;
		
		LineletBool = new bool[geometry->GetnPoint()];
		for (iPoint = 0; iPoint < geometry->GetnPoint(); iPoint ++)
			LineletBool[iPoint] = false;
		
		for (iLinelet = 0; iLinelet < nLinelet; iLinelet++) {
			for (iElem = 0; iElem < LineletPoint[iLinelet].size(); iElem++) {
				iPoint = LineletPoint[iLinelet][iElem];
				LineletBool[iPoint] = true;
			}
		}
	}
}

void CSparseMatrix::ComputeJacobiPreconditioner(const CSysVector & vec, CSysVector & prod, CGeometry *geometry, CConfig *config) {
	unsigned long iPoint, iVar, jVar, iVertex, nVertexS,
  nVertexR, nBufferS_Vector, nBufferR_Vector;
  unsigned short iMarker, MarkerS, MarkerR;
	double *Buffer_Receive = NULL, *Buffer_Send = NULL;
	int send_to, receive_from;
	
#ifndef NO_MPI
  MPI::Status status;
  MPI::Request send_request, recv_request;
#endif
  
	for (iPoint = 0; iPoint < nPoint; iPoint++) {
		for (iVar = 0; iVar < nVar; iVar++) {
			prod[(const unsigned int)(iPoint*nVar+iVar)] = 0;
			for (jVar = 0; jVar < nVar; jVar++)
				prod[(const unsigned int)(iPoint*nVar+iVar)] += invM[(const unsigned int)(iPoint*nVar*nVar+iVar*nVar+jVar)]*vec[(const unsigned int)(iPoint*nVar+jVar)];
		}
	}
  
  /*--- MPI Parallelization ---*/
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) {
    
		if ((config->GetMarker_All_Boundary(iMarker) == SEND_RECEIVE) &&
        (config->GetMarker_All_SendRecv(iMarker) > 0)) {
			
			MarkerS = iMarker;  MarkerR = iMarker+1;
      
      send_to = config->GetMarker_All_SendRecv(MarkerS)-1;
			receive_from = abs(config->GetMarker_All_SendRecv(MarkerR))-1;
			
			nVertexS = geometry->nVertex[MarkerS];  nVertexR = geometry->nVertex[MarkerR];
			nBufferS_Vector = nVertexS*nVar;        nBufferR_Vector = nVertexR*nVar;
      
      /*--- Allocate Receive and send buffers  ---*/
      Buffer_Receive = new double [nBufferR_Vector];
      Buffer_Send = new double[nBufferS_Vector];
      
      /*--- Copy the solution that should be sended ---*/
      for (iVertex = 0; iVertex < nVertexS; iVertex++) {
        iPoint = geometry->vertex[MarkerS][iVertex]->GetNode();
        for (iVar = 0; iVar < nVar; iVar++)
          Buffer_Send[iVertex*nVar+iVar] = prod[(const unsigned int)(iPoint*nVar+iVar)];
      }
      
#ifndef NO_MPI
      
//      /*--- Send/Receive using non-blocking communications ---*/
//      send_request = MPI::COMM_WORLD.Isend(Buffer_Send, nBufferS_Vector, MPI::DOUBLE, 0, send_to);
//      recv_request = MPI::COMM_WORLD.Irecv(Buffer_Receive, nBufferR_Vector, MPI::DOUBLE, 0, receive_from);
//      send_request.Wait(status);
//      recv_request.Wait(status);
      
      /*--- Send/Receive information using Sendrecv ---*/
      MPI::COMM_WORLD.Sendrecv(Buffer_Send, nBufferS_Vector, MPI::DOUBLE, send_to, 0,
                               Buffer_Receive, nBufferR_Vector, MPI::DOUBLE, receive_from, 0);
      
#else
      
      /*--- Receive information without MPI ---*/
      for (iVertex = 0; iVertex < nVertexR; iVertex++) {
        iPoint = geometry->vertex[MarkerR][iVertex]->GetNode();
        for (iVar = 0; iVar < nVar; iVar++)
          Buffer_Receive[iVar*nVertexR+iVertex] = Buffer_Send[iVar*nVertexR+iVertex];
      }
      
#endif
      
      /*--- Deallocate send buffer ---*/
      delete [] Buffer_Send;
      
      /*--- Do the coordinate transformation ---*/
      for (iVertex = 0; iVertex < nVertexR; iVertex++) {
        
        /*--- Find point and its type of transformation ---*/
        iPoint = geometry->vertex[MarkerR][iVertex]->GetNode();
        
        /*--- Copy transformed conserved variables back into buffer. ---*/
        for (iVar = 0; iVar < nVar; iVar++) {
          prod[(const unsigned int)(iPoint*nVar+iVar)] = Buffer_Receive[iVertex*nVar+iVar];
        }
        
      }
      
      /*--- Deallocate receive buffer ---*/
      delete [] Buffer_Receive;
      
    }
    
	}
	
}

void CSparseMatrix::ComputeLineletPreconditioner(const CSysVector & vec, CSysVector & prod, CGeometry *geometry, CConfig *config) {
	unsigned long iVar, jVar, nElem = 0, iLinelet, im1Point, iPoint, ip1Point, iElem, iVertex, nVertexS,
  nVertexR, nBufferS_Vector, nBufferR_Vector;
  unsigned short iMarker, MarkerS, MarkerR;
	double *Buffer_Receive = NULL, *Buffer_Send = NULL;
	int send_to, receive_from;
	long iElemLoop;
	
#ifndef NO_MPI
  MPI::Status status;
  MPI::Request send_request, recv_request;
#endif
  
	nElem = LineletPoint[0].size();
	for (iLinelet = 1; iLinelet < nLinelet; iLinelet++)
		if (LineletPoint[iLinelet].size() > nElem)
			nElem = LineletPoint[iLinelet].size();
	
	/*--- Memory allocation, this should be done in the constructor ---*/
	double **UBlock = new double* [nElem];
	double **invUBlock = new double* [nElem];
	double **LBlock = new double* [nElem];
	double **yVector = new double* [nElem];
	double **zVector = new double* [nElem];
	double **rVector = new double* [nElem];
	for (iElem = 0; iElem < nElem; iElem++) {
		UBlock[iElem] = new double [nVar*nVar];
		invUBlock[iElem] = new double [nVar*nVar];
		LBlock[iElem] = new double [nVar*nVar];
		yVector[iElem] = new double [nVar];
		zVector[iElem] = new double [nVar];
		rVector[iElem] = new double [nVar];
	}
	
	double *LFBlock = new double [nVar*nVar];
	double *LyVector = new double [nVar];
	double *FzVector = new double [nVar];
	double *AuxVector = new double [nVar];
	
	/*--- Jacobi preconditioning if there is no linelet ---*/
	for (iPoint = 0; iPoint < nPoint; iPoint++)
		if (!LineletBool[iPoint])
			for (iVar = 0; iVar < nVar; iVar++) {
				prod[(const unsigned int)(iPoint*nVar+iVar)] = 0;
				for (jVar = 0; jVar < nVar; jVar++)
					prod[(const unsigned int)(iPoint*nVar+iVar)] += invM[(const unsigned int)(iPoint*nVar*nVar+iVar*nVar+jVar)]*vec[(const unsigned int)(iPoint*nVar+jVar)];
			}
	
	/*--- Solve linelet using a Thomas algorithm ---*/
	for (iLinelet = 0; iLinelet < nLinelet; iLinelet++) {
		
		nElem = LineletPoint[iLinelet].size();
		
		/*--- Copy vec vector to the new structure ---*/
		for (iElem = 0; iElem < nElem; iElem++) {
			iPoint = LineletPoint[iLinelet][iElem];
			for (iVar = 0; iVar < nVar; iVar++)
				rVector[iElem][iVar] = vec[(const unsigned int)(iPoint*nVar+iVar)];
		}
		
		/*--- Initialization (iElem = 0) ---*/
		iPoint = LineletPoint[iLinelet][0];
		GetBlock(iPoint, iPoint);
		for (iVar = 0; iVar < nVar; iVar++) {
			yVector[0][iVar] = rVector[0][iVar];
			for (jVar = 0; jVar < nVar; jVar++)
				UBlock[0][iVar*nVar+jVar] = block[iVar*nVar+jVar];
		}
		
		/*--- Main loop (without iElem = 0) ---*/
		for (iElem = 1; iElem < nElem; iElem++) {
			
			im1Point = LineletPoint[iLinelet][iElem-1];
			iPoint = LineletPoint[iLinelet][iElem];
			
			InverseBlock(UBlock[iElem-1], invUBlock[iElem-1]);
			GetBlock(iPoint, im1Point); GetMultBlockBlock(LBlock[iElem], block, invUBlock[iElem-1]);
			GetBlock(im1Point, iPoint); GetMultBlockBlock(LFBlock, LBlock[iElem], block);
			GetBlock(iPoint, iPoint); GetSubsBlock(UBlock[iElem], block, LFBlock);
			
			/*--- Forward substituton ---*/
			GetMultBlockVector(LyVector, LBlock[iElem], yVector[iElem-1]);
			GetSubsVector(yVector[iElem], rVector[iElem], LyVector);
		}
		
		/*--- Backward substituton ---*/
		im1Point = LineletPoint[iLinelet][nElem-1];
		InverseBlock(UBlock[nElem-1], invUBlock[nElem-1]);
		GetMultBlockVector(zVector[nElem-1], invUBlock[nElem-1], yVector[nElem-1]);
		
		for (iElemLoop = nElem-2; iElemLoop >= 0; iElemLoop--) {
			iPoint = LineletPoint[iLinelet][iElemLoop];
			ip1Point = LineletPoint[iLinelet][iElemLoop+1];
			GetBlock(iPoint, ip1Point); GetMultBlockVector(FzVector, block, zVector[iElemLoop+1]);
			GetSubsVector(AuxVector, yVector[iElemLoop], FzVector);
			GetMultBlockVector(zVector[iElemLoop], invUBlock[iElemLoop], AuxVector);
		}
		
		/*--- Copy zVector to the prod vector ---*/
		for (iElem = 0; iElem < nElem; iElem++) {
			iPoint = LineletPoint[iLinelet][iElem];
			for (iVar = 0; iVar < nVar; iVar++)
				prod[(const unsigned int)(iPoint*nVar+iVar)] = zVector[iElem][iVar];
		}
	}
  
  /*--- MPI Parallelization ---*/
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) {
    
		if ((config->GetMarker_All_Boundary(iMarker) == SEND_RECEIVE) &&
        (config->GetMarker_All_SendRecv(iMarker) > 0)) {
			
			MarkerS = iMarker;  MarkerR = iMarker+1;
      
      send_to = config->GetMarker_All_SendRecv(MarkerS)-1;
			receive_from = abs(config->GetMarker_All_SendRecv(MarkerR))-1;
			
			nVertexS = geometry->nVertex[MarkerS];  nVertexR = geometry->nVertex[MarkerR];
			nBufferS_Vector = nVertexS*nVar;        nBufferR_Vector = nVertexR*nVar;
      
      /*--- Allocate Receive and send buffers  ---*/
      Buffer_Receive = new double [nBufferR_Vector];
      Buffer_Send = new double[nBufferS_Vector];
      
      /*--- Copy the solution that should be sended ---*/
      for (iVertex = 0; iVertex < nVertexS; iVertex++) {
        iPoint = geometry->vertex[MarkerS][iVertex]->GetNode();
        for (iVar = 0; iVar < nVar; iVar++)
          Buffer_Send[iVertex*nVar+iVar] = prod[(const unsigned int)(iPoint*nVar+iVar)];
      }
      
#ifndef NO_MPI
      
//      /*--- Send/Receive using non-blocking communications ---*/
//      send_request = MPI::COMM_WORLD.Isend(Buffer_Send, nBufferS_Vector, MPI::DOUBLE, 0, send_to);
//      recv_request = MPI::COMM_WORLD.Irecv(Buffer_Receive, nBufferR_Vector, MPI::DOUBLE, 0, receive_from);
//      send_request.Wait(status);
//      recv_request.Wait(status);
      
      /*--- Send/Receive information using Sendrecv ---*/
      MPI::COMM_WORLD.Sendrecv(Buffer_Send, nBufferS_Vector, MPI::DOUBLE, send_to, 0,
                               Buffer_Receive, nBufferR_Vector, MPI::DOUBLE, receive_from, 0);
      
#else
      
      /*--- Receive information without MPI ---*/
      for (iVertex = 0; iVertex < nVertexR; iVertex++) {
        iPoint = geometry->vertex[MarkerR][iVertex]->GetNode();
        for (iVar = 0; iVar < nVar; iVar++)
          Buffer_Receive[iVar*nVertexR+iVertex] = Buffer_Send[iVar*nVertexR+iVertex];
      }
      
#endif
      
      /*--- Deallocate send buffer ---*/
      delete [] Buffer_Send;
      
      /*--- Do the coordinate transformation ---*/
      for (iVertex = 0; iVertex < nVertexR; iVertex++) {
        
        /*--- Find point and its type of transformation ---*/
        iPoint = geometry->vertex[MarkerR][iVertex]->GetNode();
        
        /*--- Copy transformed conserved variables back into buffer. ---*/
        for (iVar = 0; iVar < nVar; iVar++) {
          prod[(const unsigned int)(iPoint*nVar+iVar)] = Buffer_Receive[iVertex*nVar+iVar];
        }
        
      }
      
      /*--- Deallocate receive buffer ---*/
      delete [] Buffer_Receive;
      
    }
    
	}
  
}

void CSparseMatrix::ComputeIdentityPreconditioner(const CSysVector & vec, CSysVector & prod, CGeometry *geometry, CConfig *config) {
	unsigned long iPoint, iVar, iVertex, nVertexS,
  nVertexR, nBufferS_Vector, nBufferR_Vector;
  unsigned short iMarker, MarkerS, MarkerR;
	double *Buffer_Receive = NULL, *Buffer_Send = NULL;
	int send_to, receive_from;
	
#ifndef NO_MPI
  MPI::Status status;
  MPI::Request send_request, recv_request;
#endif
  
	for (iPoint = 0; iPoint < nPoint; iPoint++) {
		for (iVar = 0; iVar < nVar; iVar++) {
      prod[(const unsigned int)(iPoint*nVar+iVar)] = vec[(const unsigned int)(iPoint*nVar+iVar)];
		}
	}
  
  /*--- MPI Parallelization ---*/
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) {
    
		if ((config->GetMarker_All_Boundary(iMarker) == SEND_RECEIVE) &&
        (config->GetMarker_All_SendRecv(iMarker) > 0)) {
			
			MarkerS = iMarker;  MarkerR = iMarker+1;
      
      send_to = config->GetMarker_All_SendRecv(MarkerS)-1;
			receive_from = abs(config->GetMarker_All_SendRecv(MarkerR))-1;
			
			nVertexS = geometry->nVertex[MarkerS];  nVertexR = geometry->nVertex[MarkerR];
			nBufferS_Vector = nVertexS*nVar;        nBufferR_Vector = nVertexR*nVar;
      
      /*--- Allocate Receive and send buffers  ---*/
      Buffer_Receive = new double [nBufferR_Vector];
      Buffer_Send = new double[nBufferS_Vector];
      
      /*--- Copy the solution that should be sended ---*/
      for (iVertex = 0; iVertex < nVertexS; iVertex++) {
        iPoint = geometry->vertex[MarkerS][iVertex]->GetNode();
        for (iVar = 0; iVar < nVar; iVar++)
          Buffer_Send[iVertex*nVar+iVar] = prod[(const unsigned int)(iPoint*nVar+iVar)];
      }
      
#ifndef NO_MPI
      
//      /*--- Send/Receive using non-blocking communications ---*/
//      send_request = MPI::COMM_WORLD.Isend(Buffer_Send, nBufferS_Vector, MPI::DOUBLE, 0, send_to);
//      recv_request = MPI::COMM_WORLD.Irecv(Buffer_Receive, nBufferR_Vector, MPI::DOUBLE, 0, receive_from);
//      send_request.Wait(status);
//      recv_request.Wait(status);
      
      /*--- Send/Receive information using Sendrecv ---*/
      MPI::COMM_WORLD.Sendrecv(Buffer_Send, nBufferS_Vector, MPI::DOUBLE, send_to, 0,
                               Buffer_Receive, nBufferR_Vector, MPI::DOUBLE, receive_from, 0);
      
#else
      
      /*--- Receive information without MPI ---*/
      for (iVertex = 0; iVertex < nVertexR; iVertex++) {
        iPoint = geometry->vertex[MarkerR][iVertex]->GetNode();
        for (iVar = 0; iVar < nVar; iVar++)
          Buffer_Receive[iVar*nVertexR+iVertex] = Buffer_Send[iVar*nVertexR+iVertex];
      }
      
#endif
      
      /*--- Deallocate send buffer ---*/
      delete [] Buffer_Send;
      
      /*--- Do the coordinate transformation ---*/
      for (iVertex = 0; iVertex < nVertexR; iVertex++) {
        
        /*--- Find point and its type of transformation ---*/
        iPoint = geometry->vertex[MarkerR][iVertex]->GetNode();
        
        /*--- Copy transformed conserved variables back into buffer. ---*/
        for (iVar = 0; iVar < nVar; iVar++) {
          prod[(const unsigned int)(iPoint*nVar+iVar)] = Buffer_Receive[iVertex*nVar+iVar];
        }
        
      }
      
      /*--- Deallocate receive buffer ---*/
      delete [] Buffer_Receive;
      
    }
    
	}
  
}

void CSparseMatrix::LU_SGSIteration(double* b, double* x_n, CGeometry *geometry, CConfig *config) {
	unsigned long iPoint, iVar;
  
  /*--- There are two approaches to the parallelization (AIAA-2000-0927):
   1. Use a special scheduling algorithm which enables data parallelism by regrouping edges. This method has the advantage of producing exactly the same result as the single processor case, but it suffers from severe overhead penalties for parallel loop initiation, heavy interprocessor communications and poor load balance.
   2. Split the computational domain into several nonoverlapping regions according to the number of processors, and apply the SGS method inside of each region with (or without) some special interprocessor boundary treatment. This approach may suffer from convergence degradation but takes advantage of minimal parallelization overhead and good load balance. ---*/
	
	/*--- First part of the symmetric iteration: (D+L).x* = b ---*/
	for (iPoint = 0; iPoint < nPointDomain; iPoint++) {
		LowerProduct(x_n, iPoint);                                        // Compute L.x*
		for (iVar = 0; iVar < nVar; iVar++)
			aux_vector[iVar] = b[iPoint*nVar+iVar] - prod_row_vector[iVar]; // Compute aux_vector = b - L.x*
		Gauss_Elimination(iPoint, aux_vector);                            // Solve D.x* = aux_vector
		for (iVar = 0; iVar < nVar; iVar++)
			x_n[iPoint*nVar+iVar] = aux_vector[iVar];                       // Assesing x* = solution
	}

	/*--- Inner send-receive operation the solution vector ---*/
	SendReceive_Solution(x_n, geometry, config);
	
	/*--- Second part of the symmetric iteration: (D+U).x_(1) = D.x* ---*/
	for (iPoint = nPointDomain-1; (int)iPoint >= 0; iPoint--) {
		DiagonalProduct(x_n, iPoint);                 // Compute D.x*
		for (iVar = 0; iVar < nVar; iVar++)
			aux_vector[iVar] = prod_row_vector[iVar];   // Compute aux_vector = D.x*
		UpperProduct(x_n, iPoint);                    // Compute U.x_(n+1)
		for (iVar = 0; iVar < nVar; iVar++)
			aux_vector[iVar] -= prod_row_vector[iVar];  // Compute aux_vector = D.x*-U.x_(n+1)
		Gauss_Elimination(iPoint, aux_vector);        // Solve D.x* = aux_vector
		for (iVar = 0; iVar < nVar; iVar++)
			x_n[iPoint*nVar + iVar] = aux_vector[iVar]; // Assesing x_(1) = solution
	}
  
  /*--- Final send-receive operation the solution vector (redundant in CFD simulations) ---*/
	SendReceive_Solution(x_n, geometry, config);

}

void CSparseMatrix::SGSSolution(double* b, double* x_i, double tol, int max_it, 
		bool monitoring, CGeometry *geometry, CConfig *config) {
	int iter;
	unsigned long iPoint, iVar;
	double norm_1 = 0.0, norm_2 = 0.0, norm = 0.0, my_norm;

#ifndef NO_MPI
	int rank = MPI::COMM_WORLD.Get_rank();
#else
	int rank = MASTER_NODE;
#endif

	for (iter = 0; iter < max_it; iter++) {

		norm_1 = 0.0; norm_2 = 0.0;

		/*--- First part of the symmetric iteration: (D+L).x* = b-U.x_n ---*/
		for (iPoint = 0; iPoint < nPointDomain; iPoint++) {
			UpperProduct(x_i,iPoint);  // compute U.x_n (result written in prod_row_vector)
			for (iVar = 0; iVar < nVar; iVar++)
				aux_vector[iVar] = b[iPoint*nVar+iVar] - prod_row_vector[iVar]; // compute aux_vector = b-U.x_n
			LowerProduct(x_i,iPoint);  // compute L.x* (idem)
			for (iVar = 0; iVar < nVar; iVar++)
				aux_vector[iVar] -= prod_row_vector[iVar]; // compute aux_vector = b-U.x_n-L.x*
			Gauss_Elimination(iPoint, aux_vector); // solve D.x* = aux_vector
			for (iVar = 0; iVar < nVar; iVar++) {
				norm_1 += (x_i[iPoint*nVar+iVar]-aux_vector[iVar])*(x_i[iPoint*nVar+iVar]-aux_vector[iVar]);
				x_i[iPoint*nVar+iVar] = aux_vector[iVar]; // assesing x* = solution
			}
		}
		norm_1 = sqrt(norm_1);

		/*--- Send-Receive the solution vector (MPI) ---*/
		SendReceive_Solution(x_i, geometry, config);

		/*--- Second part of the symmetric iteration: (D+U).x_(n+1) = b-L.x* ---*/
		for (iPoint = nPointDomain-1; (int)iPoint >= 0; iPoint--) {
			LowerProduct(x_i,iPoint);  // compute L.x* (result written in prod_row_vector)
			for (iVar = 0; iVar < nVar; iVar++)
				aux_vector[iVar] = b[iPoint*nVar+iVar] - prod_row_vector[iVar]; // compute aux_vector = b-L.x*
			UpperProduct(x_i,iPoint);  // compute U.x_(n+1) (idem)
			for (iVar = 0; iVar < nVar; iVar++)
				aux_vector[iVar] -= prod_row_vector[iVar]; // compute aux_vector = b-L.x*-U.x_(n+1)
			Gauss_Elimination(iPoint, aux_vector); // solve D.x_(n+1) = aux_vector
			for (iVar = 0; iVar < nVar; iVar++) {
				norm_2 += (x_i[iPoint*nVar+iVar]-aux_vector[iVar])*(x_i[iPoint*nVar+iVar]-aux_vector[iVar]);
				x_i[iPoint*nVar+iVar] = aux_vector[iVar]; // assesing x* = solution
			}
			if (iPoint == 0) break;
		}
		norm_2 = sqrt(norm_2);

		my_norm = (norm_1+norm_2);

#ifndef NO_MPI
		MPI::COMM_WORLD.Allreduce(&my_norm, &norm, 1, MPI::DOUBLE, MPI::MIN); 
#else
		norm = my_norm;
#endif

		if ((monitoring == true) && (iter%1 == 0) && (rank == MASTER_NODE) && (iter > 0))
			cout << "SGS-Solution:: Iteration = " << iter << " ; Norm of the residual: " << norm << endl;

		if (norm < tol) break;

	}

	if ((monitoring == true) && (rank == MASTER_NODE) && (iter%100 == 0))
		cout << "SGS-Solution => Iter: " << iter << " | Norm Res: " << norm << endl;
	return;
}

void CSparseMatrix::SetLin_Sol_History_Header(ofstream *LinConvHist_file, CConfig *config) {
	char cstr[200], buffer[50];

	/*--- Write file name with extension ---*/
	strcpy (cstr, config->GetLin_Conv_FileName().data());
	if (config->GetOutput_FileFormat() == PARAVIEW)  sprintf (buffer, ".csv");
	if (config->GetOutput_FileFormat() == TECPLOT)  sprintf (buffer, ".plt");
	strcat(cstr,buffer);

	LinConvHist_file->open(cstr, ios::out);
	LinConvHist_file->precision(15);

	char begin[]= "\"Iteration\"";

	char lin_resid[]= ",\"Residual\"";

	char end[]= "\n";

	if (config->GetOutput_FileFormat() == TECPLOT) {
		char title[]= "TITLE = \"SU2 Simulation\"";
		char variables[]= "VARIABLES = ";
		LinConvHist_file[0] << title << endl;
		LinConvHist_file[0] << variables;
	}

	LinConvHist_file[0] << begin << lin_resid << end;

	if (config->GetOutput_FileFormat() == TECPLOT) {
		char zone[]= "ZONE T= \"Linear solver convergence history\"";
		LinConvHist_file[0] << zone << endl;
	}


}

void CSparseMatrix::SetLin_Sol_History_Iter(ofstream *LinConvHist_file, CConfig *config,
		unsigned long iExtIter, double linear_resid) {

	/*--- WARNING: These buffers have hard-coded lengths. Note that you
	 may have to adjust them to be larger if adding more entries. ---*/
	char begin[200], end[200];

	bool write_heads = ((iExtIter % (config->GetWrt_Con_Freq()*20)) == 0);


	/*--- Write the begining of the history file ---*/
	sprintf (begin, "%12d", int(iExtIter));

	/*--- Write the end of the history file ---*/
	sprintf (end, "\n");

	if (write_heads) {
		cout << endl << " Iter" << "     Residual" << endl;
	}

	/*--- Write the solution on the screen and history file ---*/

	LinConvHist_file[0] << begin << linear_resid;
	LinConvHist_file[0] << end;

	cout.precision(6);
	cout.setf(ios::fixed,ios::floatfield);
	cout.width(5); cout << iExtIter;
	cout.width(13); cout << linear_resid;

	cout << endl;

	cout.unsetf(ios::fixed);
	
}

void CSparseMatrix::PrecondVectorProduct(double* vec, double* prod, double nPoint) {
	unsigned long iPoint, iVar, jVar;
	
	for (iPoint = 0; iPoint < nPoint; iPoint++) {
		for (iVar = 0; iVar < nVar; iVar++) {
			prod[iPoint*nVar+iVar] = 0;
			for (jVar = 0; jVar < nVar; jVar++)
				prod[iPoint*nVar+iVar] += invM[iPoint*nVar*nVar+iVar*nVar+jVar]*vec[iPoint*nVar+jVar];
		}
	}
	
}

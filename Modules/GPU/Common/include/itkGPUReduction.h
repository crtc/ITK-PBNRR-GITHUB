/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef __itkGPUReduction_h
#define __itkGPUReduction_h

#include "itkObject.h"
#include "itkGPUDataManager.h"
#include "itkGPUKernelManager.h"
#include "itkOclUtil.h"

namespace itk
{
/**
 * \class GPUReduction
 *
 * This class encapsulate the parallel reduction algorithm. An example
 * of this algorithm is to compute the sum of a long array in parallel.
 *
 * \ingroup GPUCommon
 */
template< class TElement >
class ITK_EXPORT GPUReduction :
  public Object
{
public:
  /** Standard class typedefs. */
  typedef GPUReduction               Self;
  typedef Object                     Superclass;
  typedef SmartPointer< Self >       Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(GPUReduction,
               Object);

  typedef typename GPUDataManager::Pointer GPUDataPointer;

  itkGetMacro(GPUDataManager, GPUDataPointer);
  itkGetMacro(GPUResult, TElement);
  itkGetMacro(CPUResult, TElement);

  unsigned int NextPow2( unsigned int x );

  bool isPow2(unsigned int x);

  void GetNumBlocksAndThreads(int whichKernel, int n, int maxBlocks, int maxThreads, int &blocks, int &threads);

  unsigned int GetReductionKernel(int whichKernel, int blockSize, int isPowOf2);

  void AllocateGPUInputBuffer(unsigned int size);

  void ReleaseGPUInputBuffer();

  void InitializeKernel(unsigned int size);

  TElement RandomTest();

  TElement GPUGenerateData();

  TElement CPUGenerateData(TElement *data, int size);

  TElement GPUReduce(  cl_int  n,
                       int  numThreads,
                       int  numBlocks,
                       int  maxThreads,
                       int  maxBlocks,
                       int  whichKernel,
                       bool cpuFinalReduction,
                       int  cpuFinalThreshold,
                       double* dTotalTime,
                       GPUDataPointer idata,
                       GPUDataPointer odata);

protected:
  GPUReduction();
  ~GPUReduction() {
  }
  void PrintSelf(std::ostream & os, Indent indent) const;

  /** GPU kernel manager for GPUFiniteDifferenceFunction class */
  typename GPUKernelManager::Pointer m_GPUKernelManager;
  typename GPUDataPointer m_GPUDataManager;

  /* GPU kernel handle for GPUComputeUpdate */
  int m_ReduceGPUKernelHandle;
  int m_TestGPUKernelHandle;

  unsigned int m_Size;
  bool         m_SmallBlock;

  TElement m_GPUResult, m_CPUResult;
private:
  GPUReduction(const Self &);   //purposely not implemented
  void operator=(const Self &); //purposely not implemented

};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkGPUReduction.hxx"
#endif

#endif

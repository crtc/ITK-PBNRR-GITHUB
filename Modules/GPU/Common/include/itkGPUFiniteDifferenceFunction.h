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
#ifndef __itkGPUFiniteDifferenceFunction_h
#define __itkGPUFiniteDifferenceFunction_h

#include "itkFiniteDifferenceFunction.h"
#include "itkLightObject.h"
#include "itkConstNeighborhoodIterator.h"
#include "itkVector.h"

#include "itkGPUDataManager.h"
#include "itkGPUKernelManager.h"

namespace itk
{
/**
 * \class GPUFiniteDifferenceFunction
 *
 * This is a base class of GPU finite difference function.
 * Note that unlike most GPU classes, derived class of GPUFiniteDifferenceFunction
 * does not have corresponding CPU class as its parent but only has CPU class
 * FiniteDifferenceFunction as its base class. Therefore, only members of
 * FiniteDifferenceFunction are reused by its derived GPU classes.
 *
 * \par How to use this class
 * GPUFiniteDifferenceFunction must be subclassed to add functionality for
 * GPUComputeUpdate.
 *
 * \ingroup GPUCommon
 **/
template< class TImageType, class TParentFunction = FiniteDifferenceFunction< TImageType > >
class ITK_EXPORT GPUFiniteDifferenceFunction : public TParentFunction
{
public:

  /** Standard class typedefs. */
  typedef GPUFiniteDifferenceFunction  Self;
  typedef TParentFunction              Superclass;
  typedef Superclass                   DifferenceFunctionType;
  typedef SmartPointer< Self >         Pointer;
  typedef SmartPointer< const Self >   ConstPointer;

  /** Run-time type information (and related methods) */
  itkTypeMacro(GPUFiniteDifferenceFunction, TParentFunction);

  /** Extract some parameters from the image type */
  typedef typename Superclass::ImageType        ImageType;
  typedef typename Superclass::PixelType        PixelType;
  typedef typename Superclass::PixelRealType    PixelRealType;
  typedef typename Superclass::NeighborhoodType NeighborhoodType;
  typedef typename Superclass::FloatOffsetType  FloatOffsetType;
  typedef typename Superclass::RadiusType       RadiusType;
  typedef typename Superclass::TimeStepType     TimeStepType;
  typedef typename Superclass::DefaultBoundaryConditionType DefaultBoundaryConditionType;

  /** Save image dimension. */
  itkStaticConstMacro(ImageDimension, unsigned int, ImageType::ImageDimension);

  /** GPU function to compute update buffer */
  virtual void GPUComputeUpdate( const typename TImageType::Pointer output,
                                 typename TImageType::Pointer update,
                                 void *gd) = 0;

protected:
  GPUFiniteDifferenceFunction() { m_GPUKernelManager = GPUKernelManager::New(); };
  ~GPUFiniteDifferenceFunction() {}

  void PrintSelf(std::ostream & os, Indent indent) const
  {
     Superclass::PrintSelf(os, indent);
     os << indent << "GPU Finite Difference Function" << std::endl;
  };

  /** GPU kernel manager for GPUFiniteDifferenceFunction class */
  typename GPUKernelManager::Pointer m_GPUKernelManager;

  /** GPU kernel handle for GPUComputeUpdate() */
  int m_ComputeUpdateGPUKernelHandle;

private:
  GPUFiniteDifferenceFunction(const Self &); //purposely not implemented
  void operator=(const Self &);           //purposely not implemented
};
} // end namespace itk

#endif

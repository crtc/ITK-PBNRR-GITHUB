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
#ifndef __itkGPUAnisotropicDiffusionFunction_h
#define __itkGPUAnisotropicDiffusionFunction_h

#include "itkGPUFiniteDifferenceFunction.h"

namespace itk
{
/**
 * \class GPUAnisotropicDiffusionFunction
 * This is the GPU version of AnisotropicDiffusionFunction class.
 *
 * \par How to use this class
 * This class must be subclassed to provide the GPUCalculateUpdate() methods of
 * GPUFiniteDifferenceFunction and the function
 * CalculateAverageGradientMagnitudeSquared(), which is called before each
 * iteration to recalibrate the conductance term.
 *
 * \ingroup GPUCommon
 */
template< class TImage, class TParentFunction = AnisotropicDiffusionFunction< TImage > >
class ITK_EXPORT GPUAnisotropicDiffusionFunction:
  public GPUFiniteDifferenceFunction< TImage, TParentFunction >
{
public:
  /** Standard class typedefs. */
  typedef GPUAnisotropicDiffusionFunction       Self;
  typedef GPUFiniteDifferenceFunction< TImage, TParentFunction > Superclass;
  typedef SmartPointer< Self >               Pointer;
  typedef SmartPointer< const Self >         ConstPointer;

  /** Run-time type information (and related methods) */
  itkTypeMacro(GPUAnisotropicDiffusionFunction, GPUFiniteDifferenceFunction);

  /** Inherit some parameters from the superclass type */
  typedef typename Superclass::ImageType        ImageType;
  typedef typename Superclass::PixelType        PixelType;
  typedef typename Superclass::PixelRealType    PixelrealType;
  typedef typename Superclass::RadiusType       RadiusType;
  typedef typename Superclass::NeighborhoodType NeighborhoodType;
  typedef typename Superclass::TimeStepType     TimeStepType;
  typedef typename Superclass::FloatOffsetType  FloatOffsetType;

  /** Inherit some parameters from the superclass type */
  itkStaticConstMacro(ImageDimension, unsigned int, Superclass::ImageDimension);

  /** This method is called before each iteration.  It calculates a scalar
      value that is the average of the gradient magnitude squared at each pixel
      in the output image (intermediate solution). The average gradient magnitude
      value is typically used in the anisotropic diffusion equations to
      calibrate the conductance term. */
  virtual void GPUCalculateAverageGradientMagnitudeSquared(ImageType *) = 0;

protected:
  GPUAnisotropicDiffusionFunction() {}

  ~GPUAnisotropicDiffusionFunction() {}

  void PrintSelf(std::ostream & os, Indent indent) const
  {
    Superclass::PrintSelf(os, indent);
  }

  // GPU buffer for Computing Average Squared Gradient Magnitude
  typename GPUDataManager::Pointer m_AnisotropicDiffusionFunctionGPUBuffer;
  typename GPUKernelManager::Pointer m_AnisotropicDiffusionFunctionGPUKernelManager;

  // GPU Kernel Handles
  int m_AverageGradientMagnitudeSquaredGPUKernelHandle;

private:
  GPUAnisotropicDiffusionFunction(const Self &); //purposely not implemented
  void operator=(const Self &);               //purposely not implemented

};
} // end namespace itk

#endif

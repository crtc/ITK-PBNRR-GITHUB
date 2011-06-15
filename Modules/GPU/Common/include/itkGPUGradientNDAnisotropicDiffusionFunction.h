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
#ifndef __itkGPUGradientNDAnisotropicDiffusionFunction_h
#define __itkGPUGradientNDAnisotropicDiffusionFunction_h

#include "itkNeighborhoodAlgorithm.h"
#include "itkNeighborhoodInnerProduct.h"
#include "itkDerivativeOperator.h"
#include "itkGPUScalarAnisotropicDiffusionFunction.h"
#include "itkGradientNDAnisotropicDiffusionFunction.h"

namespace itk
{
/** \class GPUGradientNDAnisotropicDiffusionFunction
 *
 * This class implements an N-dimensional version of the classic Perona-Malik
 * anisotropic diffusion equation for scalar-valued images on the GPU.  See
 * itkAnisotropicDiffusionFunction for an overview of the anisotropic diffusion
 * framework and equation.
 *
 * \par
 * The conductance term for this implementation is chosen as a function of the
 * gradient magnitude of the image at each point, reducing the strength of
 * diffusion at edge pixels.
 *
 * \f[C(\mathbf{x}) = e^{-(\frac{\parallel \nabla U(\mathbf{x}) \parallel}{K})^2}\f].
 *
 * \par
 * The numerical implementation of this equation is similar to that described
 * in the Perona-Malik paper below, but uses a more robust technique
 * for gradient magnitude estimation and has been generalized to N-dimensions.
 *
 * \par References
 * Pietro Perona and Jalhandra Malik, ``Scale-space and edge detection using
 * anisotropic diffusion,'' IEEE Transactions on Pattern Analysis Machine
 * Intelligence, vol. 12, pp. 629-639, 1990.
 *
 * \ingroup GPUCommon
 */
template< class TImage, class TParentFunction = GradientNDAnisotropicDiffusionFunction< TImage > >
class ITK_EXPORT GPUGradientNDAnisotropicDiffusionFunction:
  public GPUScalarAnisotropicDiffusionFunction< TImage, TParentFunction >
{
public:
  /** Standard class typedefs. */
  typedef GPUGradientNDAnisotropicDiffusionFunction       Self;
  typedef GPUScalarAnisotropicDiffusionFunction< TImage, TParentFunction > Superclass;
  typedef SmartPointer< Self >                         Pointer;
  typedef SmartPointer< const Self >                   ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods) */
  itkTypeMacro(GPUGradientNDAnisotropicDiffusionFunction,
               GPUScalarAnisotropicDiffusionFunction);

  /** Inherit some parameters from the superclass type. */
  typedef typename Superclass::ImageType        ImageType;
  typedef typename Superclass::PixelType        PixelType;
  typedef typename Superclass::PixelRealType    PixelRealType;
  typedef typename Superclass::TimeStepType     TimeStepType;
  typedef typename Superclass::RadiusType       RadiusType;
  typedef typename Superclass::NeighborhoodType NeighborhoodType;
  typedef typename Superclass::FloatOffsetType  FloatOffsetType;
  typedef typename Superclass::NeighborhoodSizeValueType NeighborhoodSizeValueType;

  /** Inherit some parameters from the superclass type. */
  itkStaticConstMacro(ImageDimension, unsigned int, Superclass::ImageDimension);

  /** Compute the equation value. */
  virtual void GPUComputeUpdate( typename const TImage::Pointer output, typename TImage::Pointer buffer, void *globalData );

protected:
  GPUGradientNDAnisotropicDiffusionFunction();
  ~GPUGradientNDAnisotropicDiffusionFunction() {}

  static double m_MIN_NORM;
private:
  GPUGradientNDAnisotropicDiffusionFunction(const Self &); //purposely not
                                                        // implemented
  void operator=(const Self &);                         //purposely not

  // implemented
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkGPUGradientNDAnisotropicDiffusionFunction.txx"
#endif

#endif

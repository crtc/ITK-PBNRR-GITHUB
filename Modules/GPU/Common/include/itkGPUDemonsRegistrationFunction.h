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
#ifndef __itkGPUDemonsRegistrationFunction_h
#define __itkGPUDemonsRegistrationFunction_h

#include "itkGPUPDEDeformableRegistrationFunction.h"
#include "itkPoint.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkCentralDifferenceImageFunction.h"

namespace itk
{
/**
 * \class GPUDemonsRegistrationFunction
 *
 * This class encapsulate the PDE which drives the demons registration
 * algorithm. It is used by DemonsRegistrationFilter to compute the
 * output deformation field which will map a moving image onto a
 * a fixed image.
 *
 * Non-integer moving image values are obtained by using
 * interpolation. The default interpolator is of type
 * LinearInterpolateImageFunction. The user may set other
 * interpolators via method SetMovingImageInterpolator. Note that the input
 * interpolator must derive from baseclass InterpolateImageFunction.
 *
 * This class is templated over the fixed image type, moving image type,
 * and the deformation field type.
 *
 * \warning This filter assumes that the fixed image type, moving image type
 * and deformation field type all have the same number of dimensions.
 *
 * \sa DemonsRegistrationFilter
 * \ingroup FiniteDifferenceFunctions
 */
template< class TFixedImage, class TMovingImage, class TDeformationField,
          class TParentFunction = DemonsRegistrationFunction< TFixedImage, TMovingImage, TDeformationField > >
class ITK_EXPORT GPUDemonsRegistrationFunction:
  public GPUPDEDeformableRegistrationFunction< TFixedImage,
                                            TMovingImage,
                                            TDeformationField,
                                            TParentFunction >
{
public:
  /** Standard class typedefs. */
  typedef GPUDemonsRegistrationFunction Self;
  typedef GPUPDEDeformableRegistrationFunction< TFixedImage,
                                                TMovingImage,
                                                TDeformationField,
                                                TParentFunction>  Superclass;
  typedef SmartPointer< Self >          Pointer;
  typedef SmartPointer< const Self >    ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(GPUDemonsRegistrationFunction,
               DemonsRegistrationFunction);

  /** MovingImage image type. */
  typedef typename Superclass::MovingImageType    MovingImageType;
  typedef typename Superclass::MovingImagePointer MovingImagePointer;

  /** FixedImage image type. */
  typedef typename Superclass::FixedImageType    FixedImageType;
  typedef typename Superclass::FixedImagePointer FixedImagePointer;
  typedef typename FixedImageType::IndexType     IndexType;
  typedef typename FixedImageType::SizeType      SizeType;
  typedef typename FixedImageType::SpacingType   SpacingType;

  /** Deformation field type. */
  typedef typename Superclass::DeformationFieldType DeformationFieldType;
  typedef typename Superclass::DeformationFieldTypePointer
  DeformationFieldTypePointer;

  /** Inherit some enums from the superclass. */
  itkStaticConstMacro(ImageDimension, unsigned
                      int, Superclass::ImageDimension);

  virtual void GPUComputeUpdate( const DeformationFieldTypePointer output,
                                 DeformationFieldTypePointer update,
                                 void *gd);

protected:
  GPUDemonsRegistrationFunction();
  ~GPUDemonsRegistrationFunction() {}
  void PrintSelf(std::ostream & os, Indent indent) const;

private:
  GPUDemonsRegistrationFunction(const Self &); //purposely not implemented
  void operator=(const Self &);             //purposely not implemented

  /* GPU kernel handle for GPUComputeUpdate */
  int m_ComputeUpdateGPUKernelHandle;

};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkGPUDemonsRegistrationFunction.txx"
#endif

#endif

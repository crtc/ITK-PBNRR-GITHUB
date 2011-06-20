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
#ifndef __itkGPUPDEDeformableRegistrationFunction_h
#define __itkGPUPDEDeformableRegistrationFunction_h

#include "itkFiniteDifferenceFunction.h"
#include "itkPDEDeformableRegistrationFunction.h"

namespace itk
{
/** \class GPUPDEDeformableRegistrationFunction
 *
 * This is an abstract base class for all PDE functions which drives a
 * deformable registration algorithm. It is used by
 * PDEDeformationRegistrationFilter subclasses to compute the
 * output deformation field which will map a moving image onto
 * a fixed image.
 *
 * This class is templated over the fixed image type, moving image type
 * and the deformation field type.
 *
 * \sa PDEDeformableRegistrationFilter
 * \ingroup FiniteDifferenceFunctions
 */
template< class TFixedImage, class TMovingImage, class TDeformationField,
          class TParentFunction = PDEDeformableRegistrationFunction< TFixedImage, TMovingImage, TDeformationField > >
class ITK_EXPORT GPUPDEDeformableRegistrationFunction:
  public GPUFiniteDifferenceFunction< TDeformationField, TParentFunction >
{
public:
  /** Standard class typedefs. */
  typedef GPUPDEDeformableRegistrationFunction          Self;
  typedef GPUFiniteDifferenceFunction< TDeformationField, TParentFunction > Superclass;
  typedef SmartPointer< Self >                          Pointer;
  typedef SmartPointer< const Self >                    ConstPointer;

  /** Run-time type information (and related methods) */
  itkTypeMacro(GPUPDEDeformableRegistrationFunction,
               GPUFiniteDifferenceFunction);

  /** MovingImage image type. */
  typedef TMovingImage                           MovingImageType;
  typedef typename MovingImageType::ConstPointer MovingImagePointer;

  /** FixedImage image type. */
  typedef TFixedImage                           FixedImageType;
  typedef typename FixedImageType::ConstPointer FixedImagePointer;

  /** Deformation field type. */
  typedef TDeformationField                      DeformationFieldType;
  typedef typename DeformationFieldType::Pointer DeformationFieldTypePointer;

protected:
  GPUPDEDeformableRegistrationFunction()
  {
  }

  ~GPUPDEDeformableRegistrationFunction() {}

  void PrintSelf(std::ostream & os, Indent indent) const
  {
    Superclass::PrintSelf(os, indent);
  }

private:
  GPUPDEDeformableRegistrationFunction(const Self &); //purposely not implemented
  void operator=(const Self &);                    //purposely not implemented
};
} // end namespace itk

#endif

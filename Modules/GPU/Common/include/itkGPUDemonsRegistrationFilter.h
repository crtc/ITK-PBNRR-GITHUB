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
#ifndef __itkGPUDemonsRegistrationFilter_h
#define __itkGPUDemonsRegistrationFilter_h

#include "itkOclUtil.h"
#include "itkDemonsRegistrationFilter.h"
#include "itkGPUPDEDeformableRegistrationFilter.h"
#include "itkGPUDemonsRegistrationFunction.h"
#include "itkVersion.h"
#include "itkObjectFactoryBase.h"

namespace itk
{
/** \class GPUDemonsRegistrationFilter
 *
 * \brief Deformably register two images using the demons algorithm with GPU.
 *
 * GPUDemonsRegistrationFilter implements the demons deformable algorithm that
 * register two images by computing the deformation field which will map a
 * moving image onto a fixed image.
 *
 * A deformation field is represented as a image whose pixel type is some
 * vector type with at least N elements, where N is the dimension of
 * the fixed image. The vector type must support element access via operator
 * []. It is assumed that the vector elements behave like floating point
 * scalars.
 *
 * This class is templated over the fixed image type, moving image type
 * and the deformation field type.
 *
 * The input fixed and moving images are set via methods SetFixedImage
 * and SetMovingImage respectively. An initial deformation field maybe set via
 * SetInitialDeformationField or SetInput. If no initial field is set,
 * a zero field is used as the initial condition.
 *
 * The algorithm has one parameters: the number of iteration to be performed.
 *
 * The output deformation field can be obtained via methods GetOutput
 * or GetDeformationField.
 *
 * This class make use of the finite difference solver hierarchy. Update
 * for each iteration is computed in GPUDemonsRegistrationFunction.
 *
 * \warning This filter assumes that the fixed image type, moving image type
 * and deformation field type all have the same number of dimensions.
 *
 * \sa GPUDemonsRegistrationFunction
 * \ingroup DeformableImageRegistration MultiThreaded
 */
template< class TFixedImage, class TMovingImage, class TDeformationField,
          class TParentImageFilter = itk::DemonsRegistrationFilter< TFixedImage, TMovingImage, TDeformationField >
          >
class ITK_EXPORT GPUDemonsRegistrationFilter :
  public GPUPDEDeformableRegistrationFilter< TFixedImage, TMovingImage,
                                             TDeformationField, TParentImageFilter >
{
public:
  /** Standard class typedefs. */
  typedef GPUDemonsRegistrationFilter Self;
  typedef GPUPDEDeformableRegistrationFilter< TFixedImage, TMovingImage, TDeformationField,
                                              TParentImageFilter > GPUSuperclass;
  typedef TParentImageFilter
  CPUSuperclass;
  typedef SmartPointer< Self >
  Pointer;
  typedef SmartPointer< const Self >
  ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(GPUDemonsRegistrationFilter,
               GPUPDEDeformableRegistrationFilter);

  /** Inherit types from GPUSuperclass. */
  typedef typename GPUSuperclass::TimeStepType TimeStepType;

  /** FixedImage image type. */
  typedef typename GPUSuperclass::FixedImageType    FixedImageType;
  typedef typename GPUSuperclass::FixedImagePointer FixedImagePointer;

  /** MovingImage image type. */
  typedef typename GPUSuperclass::MovingImageType    MovingImageType;
  typedef typename GPUSuperclass::MovingImagePointer MovingImagePointer;

  /** Deformation field type. */
  typedef typename GPUSuperclass::DeformationFieldType
  DeformationFieldType;
  typedef typename GPUSuperclass::DeformationFieldPointer
  DeformationFieldPointer;

  /** FiniteDifferenceFunction type. */
  typedef typename Superclass::FiniteDifferenceFunctionType
  FiniteDifferenceFunctionType;

  /** GPUDemonsRegistrationFilterFunction type. */
  typedef GPUDemonsRegistrationFunction< FixedImageType, MovingImageType,
                                         DeformationFieldType >  GPUDemonsRegistrationFunctionType;

  /** Get the metric value. The metric value is the mean square difference
   * in intensity between the fixed image and transforming moving image
   * computed over the the overlapping region between the two images.
   * This is value is only available for the previous iteration and
   * NOT the current iteration. */
  virtual double GetMetric() const;

  /** Switch between using the fixed image and moving image gradient
   * for computing the deformation field updates. */
  itkSetMacro(UseMovingImageGradient, bool);
  itkGetConstMacro(UseMovingImageGradient, bool);
  itkBooleanMacro(UseMovingImageGradient);

  /** Set/Get the threshold below which the absolute difference of
   * intensity yields a match. When the intensities match between a
   * moving and fixed image pixel, the update vector (for that
   * iteration) will be the zero vector. Default is 0.001. */
  virtual void SetIntensityDifferenceThreshold(double);

  virtual double GetIntensityDifferenceThreshold() const;

protected:
  GPUDemonsRegistrationFilter();
  ~GPUDemonsRegistrationFilter() {
  }
  void PrintSelf(std::ostream & os, Indent indent) const;

  /** Initialize the state of filter and equation before each iteration. */
  virtual void InitializeIteration();

  /** Apply update. */
  virtual void ApplyUpdate(const TimeStepType& dt);

private:
  GPUDemonsRegistrationFilter(const Self &); //purposely not implemented
  void operator=(const Self &);              //purposely not implemented

  bool m_UseMovingImageGradient;
};

/** \class GPUDemonsRegistrationFilterFactory
 *
 * \brief Object Factory implemenatation for GPUDemonsRegistrationFilter
 */
class GPUDemonsRegistrationFilterFactory : public itk::ObjectFactoryBase
{
public:
  typedef GPUDemonsRegistrationFilterFactory Self;
  typedef ObjectFactoryBase                  GPUSuperclass;
  typedef SmartPointer<Self>                 Pointer;
  typedef SmartPointer<const Self>           ConstPointer;

  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion() const {
    return ITK_SOURCE_VERSION;
  }
  const char* GetDescription() const {
    return "A Factory for GPUDemonsRegistrationFilter";
  }

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(GPUDemonsRegistrationFilterFactory, itk::ObjectFactoryBase);

  /** Register one factory of this type  */
  static void RegisterOneFactory(void)
  {
    GPUDemonsRegistrationFilterFactory::Pointer factory = GPUDemonsRegistrationFilterFactory::New();

    ObjectFactoryBase::RegisterFactory(factory);
  }

private:
  GPUDemonsRegistrationFilterFactory(const Self&); //purposely not implemented
  void operator=(const Self&);                     //purposely not implemented

#define OverrideDemonsRegistrationFilterTypeMacro(ipt,opt,dm) \
    { \
    typedef GPUImage<ipt,dm>                 InputImageType; \
    typedef GPUImage<opt,dm>                 OutputImageType; \
    typedef Vector< float, dm >              VectorPixelType; \
    typedef GPUImage<  VectorPixelType, dm > DeformationFieldType; \
    this->RegisterOverride( \
      typeid(DemonsRegistrationFilter<InputImageType,OutputImageType,DeformationFieldType>).name(), \
      typeid(GPUDemonsRegistrationFilter<InputImageType,OutputImageType,DeformationFieldType>).name(), \
      "GPU Demons Registration Filter Override", \
      true, \
      CreateObjectFunction<GPUDemonsRegistrationFilter<InputImageType,OutputImageType,DeformationFieldType> >::New() ); \
    }

  GPUDemonsRegistrationFilterFactory()
  {
    if( IsGPUAvailable() )
      {
      OverrideDemonsRegistrationFilterTypeMacro(unsigned char, unsigned char, 1);
      OverrideDemonsRegistrationFilterTypeMacro(char, char, 1);
      OverrideDemonsRegistrationFilterTypeMacro(float,float,1);
      OverrideDemonsRegistrationFilterTypeMacro(int,int,1);
      OverrideDemonsRegistrationFilterTypeMacro(unsigned int,unsigned int,1);
      OverrideDemonsRegistrationFilterTypeMacro(double,double,1);

      OverrideDemonsRegistrationFilterTypeMacro(unsigned char, unsigned char, 2);
      OverrideDemonsRegistrationFilterTypeMacro(char, char, 2);
      OverrideDemonsRegistrationFilterTypeMacro(float,float,2);
      OverrideDemonsRegistrationFilterTypeMacro(int,int,2);
      OverrideDemonsRegistrationFilterTypeMacro(unsigned int,unsigned int,2);
      OverrideDemonsRegistrationFilterTypeMacro(double,double,2);

      OverrideDemonsRegistrationFilterTypeMacro(unsigned char, unsigned char, 3);
      OverrideDemonsRegistrationFilterTypeMacro(char, char, 3);
      OverrideDemonsRegistrationFilterTypeMacro(float,float,3);
      OverrideDemonsRegistrationFilterTypeMacro(int,int,3);
      OverrideDemonsRegistrationFilterTypeMacro(unsigned int,unsigned int,3);
      OverrideDemonsRegistrationFilterTypeMacro(double,double,3);
      }
  }

};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkGPUDemonsRegistrationFilter.hxx"
#endif

#endif

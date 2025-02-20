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
#include "itkGPUReduction.h"

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
template< class TFixedImage, class TMovingImage, class TDeformationField >
class ITK_EXPORT GPUDemonsRegistrationFunction :
  public GPUPDEDeformableRegistrationFunction< TFixedImage,
                                               TMovingImage,
                                               TDeformationField >
{
public:
  /** Standard class typedefs. */
  typedef GPUDemonsRegistrationFunction Self;
  typedef GPUPDEDeformableRegistrationFunction< TFixedImage, TMovingImage,
                                                TDeformationField>                  Superclass;
  typedef SmartPointer< Self >       Pointer;
  typedef SmartPointer< const Self > ConstPointer;

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

  /** Inherit some enums from the superclass. */
  typedef typename Superclass::PixelType        PixelType;
  typedef typename Superclass::RadiusType       RadiusType;
  typedef typename Superclass::NeighborhoodType NeighborhoodType;
  typedef typename Superclass::FloatOffsetType  FloatOffsetType;
  typedef typename Superclass::TimeStepType     TimeStepType;

  /** Interpolator type. */
  typedef double                                                          CoordRepType;
  typedef InterpolateImageFunction< MovingImageType, CoordRepType >       InterpolatorType;
  typedef typename InterpolatorType::Pointer                              InterpolatorPointer;
  typedef typename InterpolatorType::PointType                            PointType;
  typedef LinearInterpolateImageFunction< MovingImageType, CoordRepType > DefaultInterpolatorType;

  /** Covariant vector type. */
  typedef CovariantVector< double, itkGetStaticConstMacro(ImageDimension) > CovariantVectorType;

  /** Fixed image gradient calculator type. */
  typedef CentralDifferenceImageFunction< FixedImageType > GradientCalculatorType;
  typedef typename GradientCalculatorType::Pointer         GradientCalculatorPointer;

  /** Moving image gradient calculator type. */
  typedef CentralDifferenceImageFunction< MovingImageType, CoordRepType >
  MovingImageGradientCalculatorType;
  typedef typename MovingImageGradientCalculatorType::Pointer
  MovingImageGradientCalculatorPointer;

  /** GPU data pointer type. */
  typedef GPUDataManager::Pointer GPUDataPointer;

  /** Set the moving image interpolator. */
  void SetMovingImageInterpolator(InterpolatorType *ptr)
  {
    m_MovingImageInterpolator = ptr;
  }

  /** Get the moving image interpolator. */
  InterpolatorType * GetMovingImageInterpolator(void)
  {
    return m_MovingImageInterpolator;
  }

  /** This class uses a constant timestep of 1. */
  virtual TimeStepType ComputeGlobalTimeStep( void *itkNotUsed(GlobalData) )
  const
  {
    return m_TimeStep;
  }

  /** Return a pointer to a global data structure that is passed to
   * this object from the solver at each calculation.  */
  virtual void * GetGlobalDataPointer() const
  {
    GlobalDataStruct *global = new GlobalDataStruct();

    global->m_SumOfSquaredDifference  = 0.0;
    global->m_NumberOfPixelsProcessed = 0L;
    global->m_SumOfSquaredChange      = 0;
    return global;
  }

  /** Release memory for global data structure. */
  virtual void ReleaseGlobalDataPointer(void *GlobalData) const;

  /** Set the object's state before each iteration. */
  virtual void InitializeIteration();

  /** This method is called by a finite difference solver image filter at
   * each pixel that does not lie on a data set boundary */
  virtual PixelType  ComputeUpdate( const NeighborhoodType & neighborhood,
                                    void *globalData,
                                    const FloatOffsetType & offset =
                                      FloatOffsetType(0.0) );

  virtual void GPUComputeUpdate( const DeformationFieldTypePointer output,
                                 DeformationFieldTypePointer update,
                                 void *gd);

  /** Get the metric value. The metric value is the mean square difference
   * in intensity between the fixed image and transforming moving image
   * computed over the the overlapping region between the two images. */
  virtual double GetMetric() const
  {
    return m_Metric;
  }

  /** Get the rms change in deformation field. */
  virtual double GetRMSChange() const
  {
    return m_RMSChange;
  }

  /** Select if the fixed image or moving image gradient is used for
   * the computating the demon forces. The fixed image gradient is used
   * by default. */
  virtual void SetUseMovingImageGradient(bool flag)
  {
    m_UseMovingImageGradient = flag;
  }
  virtual bool GetUseMovingImageGradient() const
  {
    return m_UseMovingImageGradient;
  }

  /** Set/Get the threshold below which the absolute difference of
   * intensity yields a match. When the intensities match between a
   * moving and fixed image pixel, the update vector (for that
   * iteration) will be the zero vector. Default is 0.001. */
  virtual void SetIntensityDifferenceThreshold(double);

  virtual double GetIntensityDifferenceThreshold() const;

protected:
  GPUDemonsRegistrationFunction();
  ~GPUDemonsRegistrationFunction() {
  }
  void PrintSelf(std::ostream & os, Indent indent) const;

  /** FixedImage image neighborhood iterator type. */
  typedef ConstNeighborhoodIterator< FixedImageType >
  FixedImageNeighborhoodIteratorType;

  /** A global data type for this class of equation. Used to store
   * information for computing the metric. */
  struct GlobalDataStruct {
    double m_SumOfSquaredDifference;
    SizeValueType m_NumberOfPixelsProcessed;
    double m_SumOfSquaredChange;
    };

  /* GPU kernel handle for GPUComputeUpdate */
  int m_ComputeUpdateGPUKernelHandle;
private:
  GPUDemonsRegistrationFunction(const Self &); //purposely not implemented
  void operator=(const Self &);                //purposely not implemented

  /** Cache fixed image information. */
  //SpacingType                  m_FixedImageSpacing;
  //PointType                    m_FixedImageOrigin;
  PixelType m_ZeroUpdateReturn;
  double    m_Normalizer;

  /** Function to compute derivatives of the fixed image. */
  GradientCalculatorPointer m_FixedImageGradientCalculator;

  /** Function to compute derivatives of the moving image. */
  MovingImageGradientCalculatorPointer m_MovingImageGradientCalculator;
  bool                                 m_UseMovingImageGradient;

  /** Function to interpolate the moving image. */
  InterpolatorPointer m_MovingImageInterpolator;

  /** The global timestep. */
  TimeStepType m_TimeStep;

  /** Threshold below which the denominator term is considered zero. */
  double m_DenominatorThreshold;

  /** Threshold below which two intensity value are assumed to match. */
  double m_IntensityDifferenceThreshold;

  /** The metric value is the mean square difference in intensity between
   * the fixed image and transforming moving image computed over the
   * the overlapping region between the two images. */
  mutable double        m_Metric;
  mutable double        m_SumOfSquaredDifference;
  mutable SizeValueType m_NumberOfPixelsProcessed;
  mutable double        m_RMSChange;
  mutable double        m_SumOfSquaredChange;

  mutable GPUReduction<int>::Pointer   m_GPUPixelCounter;
  mutable GPUReduction<float>::Pointer m_GPUSquaredChange;
  mutable GPUReduction<float>::Pointer m_GPUSquaredDifference;

  /** Mutex lock to protect modification to metric. */
  mutable SimpleFastMutexLock m_MetricCalculationLock;

};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkGPUDemonsRegistrationFunction.hxx"
#endif

#endif

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
#ifndef __itkGPUBinaryThresholdImageFilter_h
#define __itkGPUBinaryThresholdImageFilter_h

#include "itkOclUtil.h"
#include "itkGPUFunctorBase.h"
#include "itkGPUKernelManager.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkGPUUnaryFunctorImageFilter.h"

namespace itk
{
/** \class GPUBinaryThresholdImageFilter
 *
 * \brief GPU version of binary threshold image filter.
 *
 * \ingroup GPUCommon
 */
namespace Functor
{
template< class TInput, class TOutput >
class GPUBinaryThreshold : public GPUFunctorBase
{
public:
  GPUBinaryThreshold()
  {
    m_LowerThreshold = NumericTraits< TInput >::NonpositiveMin();
    m_UpperThreshold = NumericTraits< TInput >::max();
    m_OutsideValue   = NumericTraits< TOutput >::Zero;
    m_InsideValue    = NumericTraits< TOutput >::max();
  }

  ~GPUBinaryThreshold() {
  }

  void SetLowerThreshold(const TInput & thresh)
  {
    m_LowerThreshold = thresh;
  }
  void SetUpperThreshold(const TInput & thresh)
  {
    m_UpperThreshold = thresh;
  }
  void SetInsideValue(const TOutput & value)
  {
    m_InsideValue = value;
  }
  void SetOutsideValue(const TOutput & value)
  {
    m_OutsideValue = value;
  }

  /** Setup GPU kernel arguments for this functor.
   * Returns current argument index to set additional arguments in the GPU kernel */
  int SetGPUKernelArguments(GPUKernelManager::Pointer KernelManager, int KernelHandle)
  {
    KernelManager->SetKernelArg(KernelHandle, 0, sizeof(TInput), &(m_LowerThreshold) );
    KernelManager->SetKernelArg(KernelHandle, 1, sizeof(TInput), &(m_UpperThreshold) );
    KernelManager->SetKernelArg(KernelHandle, 2, sizeof(TOutput), &(m_InsideValue) );
    KernelManager->SetKernelArg(KernelHandle, 3, sizeof(TOutput), &(m_OutsideValue) );
    return 4;
  }

private:
  TInput  m_LowerThreshold;
  TInput  m_UpperThreshold;
  TOutput m_InsideValue;
  TOutput m_OutsideValue;
};
} // end of namespace Functor

/** GPUBinaryThresholdImageFilter class definition */
template< class TInputImage, class TOutputImage >
class ITK_EXPORT GPUBinaryThresholdImageFilter :
  public
  GPUUnaryFunctorImageFilter< TInputImage, TOutputImage,
                              Functor::GPUBinaryThreshold<
                                typename TInputImage::PixelType,
                                typename TOutputImage::PixelType >,
                              BinaryThresholdImageFilter<TInputImage, TOutputImage> >
{
public:
  /** Standard class typedefs. */
  typedef GPUBinaryThresholdImageFilter Self;
  typedef GPUUnaryFunctorImageFilter< TInputImage, TOutputImage,
                                      Functor::GPUBinaryThreshold<
                                        typename TInputImage::PixelType,
                                        typename TOutputImage::PixelType >,
                                      BinaryThresholdImageFilter<TInputImage, TOutputImage> > GPUSuperclass;
  typedef BinaryThresholdImageFilter<TInputImage, TOutputImage> CPUSuperclass;
  typedef SmartPointer< Self >                                  Pointer;
  typedef SmartPointer< const Self >                            ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(GPUBinaryThresholdImageFilter, GPUUnaryFunctorImageFilter);

  /** Pixel types. */
  typedef typename TInputImage::PixelType  InputPixelType;
  typedef typename TOutputImage::PixelType OutputPixelType;

  /** Type of DataObjects to use for scalar inputs */
  typedef SimpleDataObjectDecorator< InputPixelType > InputPixelObjectType;
protected:
  GPUBinaryThresholdImageFilter();
  virtual ~GPUBinaryThresholdImageFilter() {
  }

  /** This method is used to set the state of the filter before
   * multi-threading. */
  //virtual void BeforeThreadedGenerateData();

  /** Unlike CPU version, GPU version of binary threshold filter is not
    multi-threaded */
  virtual void GPUGenerateData();

private:
  GPUBinaryThresholdImageFilter(const Self &); //purposely not implemented
  void operator=(const Self &);                //purposely not implemented

};

/** Object Factory implemenatation for GPUBinaryThresholdImageFilter */
class GPUBinaryThresholdImageFilterFactory : public ObjectFactoryBase
{
public:
  typedef GPUBinaryThresholdImageFilterFactory Self;
  typedef ObjectFactoryBase                    Superclass;
  typedef SmartPointer<Self>                   Pointer;
  typedef SmartPointer<const Self>             ConstPointer;

  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion() const {
    return ITK_SOURCE_VERSION;
  }
  const char* GetDescription() const {
    return "A Factory for GPUBinaryThresholdImageFilter";
  }

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(GPUBinaryThresholdImageFilterFactory, itk::ObjectFactoryBase);

  /** Register one factory of this type  */
  static void RegisterOneFactory(void)
  {
    GPUBinaryThresholdImageFilterFactory::Pointer factory = GPUBinaryThresholdImageFilterFactory::New();

    itk::ObjectFactoryBase::RegisterFactory(factory);
  }

private:
  GPUBinaryThresholdImageFilterFactory(const Self&); //purposely not implemented
  void operator=(const Self&);                       //purposely not implemented

#define OverrideThresholdFilterTypeMacro(ipt,opt,dm) \
    { \
    typedef itk::Image<ipt,dm> InputImageType; \
    typedef itk::Image<opt,dm> OutputImageType; \
    this->RegisterOverride( \
      typeid(itk::BinaryThresholdImageFilter<InputImageType,OutputImageType>).name(), \
      typeid(itk::GPUBinaryThresholdImageFilter<InputImageType,OutputImageType>).name(), \
      "GPU Binary Threshold Image Filter Override", \
      true, \
      itk::CreateObjectFunction<GPUBinaryThresholdImageFilter<InputImageType,OutputImageType> >::New() ); \
    }

  GPUBinaryThresholdImageFilterFactory()
  {
    if( IsGPUAvailable() )
      {
      OverrideThresholdFilterTypeMacro(unsigned char, unsigned char, 1);
      OverrideThresholdFilterTypeMacro(char, char, 1);
      OverrideThresholdFilterTypeMacro(float,float,1);
      OverrideThresholdFilterTypeMacro(int,int,1);
      OverrideThresholdFilterTypeMacro(unsigned int,unsigned int,1);
      OverrideThresholdFilterTypeMacro(double,double,1);

      OverrideThresholdFilterTypeMacro(unsigned char, unsigned char, 2);
      OverrideThresholdFilterTypeMacro(char, char, 2);
      OverrideThresholdFilterTypeMacro(float,float,2);
      OverrideThresholdFilterTypeMacro(int,int,2);
      OverrideThresholdFilterTypeMacro(unsigned int,unsigned int,2);
      OverrideThresholdFilterTypeMacro(double,double,2);

      OverrideThresholdFilterTypeMacro(unsigned char, unsigned char, 3);
      OverrideThresholdFilterTypeMacro(char, char, 3);
      OverrideThresholdFilterTypeMacro(float,float,3);
      OverrideThresholdFilterTypeMacro(int,int,3);
      OverrideThresholdFilterTypeMacro(unsigned int,unsigned int,3);
      OverrideThresholdFilterTypeMacro(double,double,3);
      }
  }

};

} // end of namespace itk

#if ITK_TEMPLATE_TXX
#include "itkGPUBinaryThresholdImageFilter.hxx"
#endif

#endif

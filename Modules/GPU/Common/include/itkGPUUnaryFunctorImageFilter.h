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
#ifndef __itkGPUUnaryFunctorImageFilter_h
#define __itkGPUUnaryFunctorImageFilter_h

#include "itkGPUFunctorBase.h"
#include "itkGPUInPlaceImageFilter.h"
#include "itkUnaryFunctorImageFilter.h"

namespace itk
{
/** \class GPUUnaryFunctorImageFilter
 * \brief Implements pixel-wise generic operation on one image using the GPU.
 *
 * GPU version of unary functor image filter.
 * GPU Functor handles parameter setup for the GPU kernel.
 *
 * \ingroup   GPUCommon
 */
template< class TInputImage, class TOutputImage, class TFunction, class TParentImageFilter = InPlaceImageFilter< TInputImage, TOutputImage > >
class ITK_EXPORT GPUUnaryFunctorImageFilter : public GPUInPlaceImageFilter< TInputImage, TOutputImage, TParentImageFilter >
{
public:
  /** Standard class typedefs. */
  typedef GPUUnaryFunctorImageFilter                         Self;
  typedef TParentImageFilter                                 CPUSuperclass;
  typedef GPUInPlaceImageFilter< TInputImage, TOutputImage > GPUSuperclass;
  typedef SmartPointer< Self >                               Pointer;
  typedef SmartPointer< const Self >                         ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(GPUUnaryFunctorImageFilter, GPUInPlaceImageFilter);

  /** Some typedefs. */
  typedef TFunction FunctorType;

  typedef TInputImage                              InputImageType;
  typedef typename    InputImageType::ConstPointer InputImagePointer;
  typedef typename    InputImageType::RegionType   InputImageRegionType;
  typedef typename    InputImageType::PixelType    InputImagePixelType;

  typedef TOutputImage                             OutputImageType;
  typedef typename     OutputImageType::Pointer    OutputImagePointer;
  typedef typename     OutputImageType::RegionType OutputImageRegionType;
  typedef typename     OutputImageType::PixelType  OutputImagePixelType;

  FunctorType &       GetFunctor() { return m_Functor; }
  const FunctorType & GetFunctor() const { return m_Functor; }

  /** Set the functor object. */
  void SetFunctor(const FunctorType & functor)
  {
    if ( m_Functor != functor )
    {
      m_Functor = functor;
      this->Modified();
    }
  }

protected:
  GPUUnaryFunctorImageFilter() {};
  virtual ~GPUUnaryFunctorImageFilter() {}

  /** GPUUnaryFunctorImageFilter can produce an image which is a different
   * resolution than its input image.  As such, UnaryFunctorImageFilter
   * needs to provide an implementation for
   * GenerateOutputInformation() in order to inform the pipeline
   * execution model.  The original documentation of this method is
   * below.
   *
   * \sa ProcessObject::GenerateOutputInformaton()  */
  virtual void GenerateOutputInformation();

  /** GPUUnaryFunctorImageFilter can be implemented as a multithreaded filter.
   * Therefore, this implementation provides a ThreadedGPUGenerateData() routine
   * which is called for each processing thread. The output image data is
   * allocated automatically by the superclass prior to calling
   * ThreadedGPUGenerateData().  ThreadedGPUGenerateData can only write to the
   * portion of the output image specified by the parameter
   * "outputRegionForThread"
   *
   * \sa ImageToImageFilter::ThreadedGenerateData(),
   *     ImageToImageFilter::GenerateData()  */
  //void ThreadedGPUGenerateData(const OutputImageRegionType & outputRegionForThread,
  //                             int threadId);

  virtual void GPUGenerateData();

private:
  GPUUnaryFunctorImageFilter(const Self &); //purposely not implemented
  void operator=(const Self &);             //purposely not implemented

  FunctorType m_Functor;
};

} // end of namespace itk

#if ITK_TEMPLATE_TXX
#include "itkGPUUnaryFunctorImageFilter.txx"
#endif

#endif

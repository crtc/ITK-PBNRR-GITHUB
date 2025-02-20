#ifndef __itkGPUImageToImageFilter_txx
#define __itkGPUImageToImageFilter_txx

#include "itkGPUImageToImageFilter.h"

namespace itk
{

template< class TInputImage, class TOutputImage, class TParentImageFilter >
GPUImageToImageFilter< TInputImage, TOutputImage, TParentImageFilter >::GPUImageToImageFilter() : m_GPUEnabled(true)
{
  m_GPUKernelManager = GPUKernelManager::New();
}

template< class TInputImage, class TOutputImage, class TParentImageFilter >
GPUImageToImageFilter< TInputImage, TOutputImage, TParentImageFilter >::~GPUImageToImageFilter()
{
}

template< class TInputImage, class TOutputImage, class TParentImageFilter >
void
GPUImageToImageFilter< TInputImage, TOutputImage, TParentImageFilter >::PrintSelf(std::ostream & os,
                                                                                  Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "GPU: " << ( m_GPUEnabled ? "Enabled" : "Disabled" ) << std::endl;
}

template< class TInputImage, class TOutputImage, class TParentImageFilter >
void
GPUImageToImageFilter< TInputImage, TOutputImage, TParentImageFilter >::GenerateData()
{
  if( !m_GPUEnabled ) // call CPU update function
    {
    Superclass::GenerateData();
    }
  else // call GPU update function
    {
    // Call a method to allocate memory for the filter's outputs
    this->AllocateOutputs();

    GPUGenerateData();
    }
}

template< class TInputImage, class TOutputImage, class TParentImageFilter >
void
GPUImageToImageFilter< TInputImage, TOutputImage, TParentImageFilter >::GraftOutput(DataObject *output)
{
  typedef typename itk::GPUTraits< TOutputImage >::Type GPUOutputImage;
  typename GPUOutputImage::Pointer otPtr = dynamic_cast< GPUOutputImage * >( this->GetOutput() );

  otPtr->Graft( output );
}

} // end namespace itk

#endif

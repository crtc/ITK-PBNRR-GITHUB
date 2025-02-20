#ifndef __itkGPUMeanImageFilter_txx
#define __itkGPUMeanImageFilter_txx

#include "itkGPUMeanImageFilter.h"

namespace itk
{

template< class TInputImage, class TOutputImage >
GPUMeanImageFilter< TInputImage, TOutputImage >::GPUMeanImageFilter()
{
  std::ostringstream defines;

  if(TInputImage::ImageDimension > 3)
    {
    itkExceptionMacro("GPUMeanImageFilter supports 1/2/3D image.");
    }

  defines << "#define DIM_" << TInputImage::ImageDimension << "\n";
  defines << "#define PIXELTYPE ";
  GetTypenameInString( typeid ( typename TInputImage::PixelType ), defines );

  //std::string oclSrcPath = std::string ( itk_root_path ) +
  // "/Code/GPU/GPUMeanImageFilter.cl";

  std::string oclSrcPath = "./../OpenCL/GPUMeanImageFilter.cl";

  std::cout << "Defines: " << defines.str() << "Source code path: " << oclSrcPath << std::endl;

  // load and build program
  this->m_GPUKernelManager->LoadProgramFromFile( oclSrcPath.c_str(), defines.str().c_str() );

  // create kernel
  m_MeanFilterGPUKernelHandle = this->m_GPUKernelManager->CreateKernel("MeanFilter");
}

template< class TInputImage, class TOutputImage >
GPUMeanImageFilter< TInputImage, TOutputImage >::~GPUMeanImageFilter()
{

}

template< class TInputImage, class TOutputImage >
void
GPUMeanImageFilter< TInputImage, TOutputImage >::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  //os << indent << "GPU: " << ( m_GPUEnabled ? "Enabled" : "Disabled" ) <<
  // std::endl;
}

template< class TInputImage, class TOutputImage >
void
GPUMeanImageFilter< TInputImage, TOutputImage >::GPUGenerateData()
{
  typedef typename itk::GPUTraits< TInputImage >::Type  GPUInputImage;
  typedef typename itk::GPUTraits< TOutputImage >::Type GPUOutputImage;

  typename GPUInputImage::Pointer  inPtr =  dynamic_cast< GPUInputImage * >( this->ProcessObject::GetInput(0) );
  typename GPUOutputImage::Pointer otPtr =  dynamic_cast< GPUOutputImage * >( this->ProcessObject::GetOutput(0) );

  typename GPUOutputImage::SizeType outSize = otPtr->GetLargestPossibleRegion().GetSize();

  int radius[3];
  int imgSize[3];

  radius[0] = radius[1] = radius[2] = 0;
  imgSize[0] = imgSize[1] = imgSize[2] = 1;

  int ImageDim = (int)TInputImage::ImageDimension;

  for(int i=0; i<ImageDim; i++)
    {
    radius[i]  = (this->GetRadius() )[i];
    imgSize[i] = outSize[i];
    }

  size_t localSize[3], globalSize[3];
  localSize[0] = localSize[1] = localSize[2] = BLOCK_SIZE[ImageDim-1];
  for(int i=0; i<ImageDim; i++)
    {
    globalSize[i] = localSize[i]*(unsigned int)ceil( (float)outSize[i]/(float)localSize[i]); //
                                                                                             // total
                                                                                             // #
                                                                                             // of
                                                                                             // threads
    }


  // arguments set up
  int argidx = 0;
  this->m_GPUKernelManager->SetKernelArgWithImage(m_MeanFilterGPUKernelHandle, argidx++, inPtr->GetGPUDataManager() );
  this->m_GPUKernelManager->SetKernelArgWithImage(m_MeanFilterGPUKernelHandle, argidx++, otPtr->GetGPUDataManager() );

  for(int i=0; i<ImageDim; i++)
    {
    this->m_GPUKernelManager->SetKernelArg(m_MeanFilterGPUKernelHandle, argidx++, sizeof(int), &(radius[i]) );
    }

  for(int i=0; i<ImageDim; i++)
    {
    this->m_GPUKernelManager->SetKernelArg(m_MeanFilterGPUKernelHandle, argidx++, sizeof(int), &(imgSize[i]) );
    }

  // launch kernel
  this->m_GPUKernelManager->LaunchKernel( m_MeanFilterGPUKernelHandle, (int)TInputImage::ImageDimension, globalSize,
                                          localSize );
}

} // end namespace itk

#endif

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
#ifndef __itkGPUGradientNDAnisotropicDiffusionFunction_txx
#define __itkGPUGradientNDAnisotropicDiffusionFunction_txx

#include "itkNumericTraits.h"
#include "itkGPUGradientNDAnisotropicDiffusionFunction.h"

#include "itkOclUtil.h"

namespace itk
{
template< class TImage >
double GPUGradientNDAnisotropicDiffusionFunction< TImage >
::m_MIN_NORM = 1.0e-10;

template< class TImage >
GPUGradientNDAnisotropicDiffusionFunction< TImage >
::GPUGradientNDAnisotropicDiffusionFunction()
{
  unsigned int i, j;
  RadiusType   r;

  for ( i = 0; i < ImageDimension; ++i )
    {
    r[i] = 1;
    }
  this->SetRadius(r);

  // Dummy neighborhood used to set up the slices.
  Neighborhood< PixelType, ImageDimension > it;
  it.SetRadius(r);

  // Slice the neighborhood
  m_Center =  it.Size() / 2;

  for ( i = 0; i < ImageDimension; ++i )
    {
    m_Stride[i] = it.GetStride(i);
    }

  for ( i = 0; i < ImageDimension; ++i )
    {
    x_slice[i]  = std::slice(m_Center - m_Stride[i], 3, m_Stride[i]);
    }

  for ( i = 0; i < ImageDimension; ++i )
    {
    for ( j = 0; j < ImageDimension; ++j )
      {
      // For taking derivatives in the i direction that are offset one
      // pixel in the j direction.
      xa_slice[i][j] =
        std::slice( ( m_Center + m_Stride[j] ) - m_Stride[i], 3, m_Stride[i] );
      xd_slice[i][j] =
        std::slice( ( m_Center - m_Stride[j] ) - m_Stride[i], 3, m_Stride[i] );
      }
    }

  // Allocate the derivative operator.
  dx_op.SetDirection(0);  // Not relevant, will be applied in a slice-based
                          // fashion.
  dx_op.SetOrder(1);
  dx_op.CreateDirectional();

  //
  // Create GPU Kernel
  //
  std::ostringstream defines;

  if(TImage::ImageDimension > 3)
    {
    itkExceptionMacro("GPUGradientNDAnisotropicDiffusionFunction supports 1/2/3D image.");
    }

  defines << "#define DIM_" << TImage::ImageDimension << "\n";
  defines << "#define BLOCK_SIZE " << BLOCK_SIZE[TImage::ImageDimension-1] << "\n";

  defines << "#define PIXELTYPE ";
  GetTypenameInString( typeid ( typename TImage::PixelType ), defines );

  std::string oclSrcPath = "./../OpenCL/GPUGradientNDAnisotropicDiffusionFunction.cl";

  std::cout << "Defines: " << defines.str() << "Source code path: " << oclSrcPath << std::endl;

  // load and build program
  this->m_GPUKernelManager->LoadProgramFromFile( oclSrcPath.c_str(), defines.str().c_str() );

  // create kernel
  this->m_ComputeUpdateGPUKernelHandle = this->m_GPUKernelManager->CreateKernel("ComputeUpdate");
}

template< class TImage >
void
GPUGradientNDAnisotropicDiffusionFunction< TImage >
::GPUComputeUpdate( const typename TImage::Pointer output, typename TImage::Pointer buffer, void *globalData )
{
  /** Launch GPU kernel to update buffer with output
   * GPU version of ComputeUpdate() - compute entire update buffer */
  typedef typename itk::GPUTraits< TImage >::Type GPUImageType;

  typename GPUImageType::Pointer  inPtr =  dynamic_cast< GPUImageType * >( output.GetPointer() );
  typename GPUImageType::Pointer  bfPtr =  dynamic_cast< GPUImageType * >( buffer.GetPointer() );
  typename GPUImageType::SizeType outSize = bfPtr->GetLargestPossibleRegion().GetSize();

  int imgSize[3];
  imgSize[0] = imgSize[1] = imgSize[2] = 1;
  float imgScale[3];
  imgScale[0] = imgScale[1] = imgScale[2] = 1.0f;

  int ImageDim = (int)TImage::ImageDimension;

  for(int i=0; i<ImageDim; i++)
    {
    imgSize[i] = outSize[i];
    imgScale[i] = this->m_ScaleCoefficients[i];
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
  this->m_GPUKernelManager->SetKernelArgWithImage(this->m_ComputeUpdateGPUKernelHandle, argidx++,
                                                  inPtr->GetGPUDataManager() );
  this->m_GPUKernelManager->SetKernelArgWithImage(this->m_ComputeUpdateGPUKernelHandle, argidx++,
                                                  bfPtr->GetGPUDataManager() );
  this->m_GPUKernelManager->SetKernelArg(this->m_ComputeUpdateGPUKernelHandle, argidx++,
                                         sizeof(typename TImage::PixelType), &(m_K) );

  // filter scale parameter
  for(int i=0; i<ImageDim; i++)
    {
    this->m_GPUKernelManager->SetKernelArg(this->m_ComputeUpdateGPUKernelHandle, argidx++, sizeof(float), &(imgScale[i]) );
    }

  // image size
  for(int i=0; i<ImageDim; i++)
    {
    this->m_GPUKernelManager->SetKernelArg(this->m_ComputeUpdateGPUKernelHandle, argidx++, sizeof(int), &(imgSize[i]) );
    }

  // launch kernel
  this->m_GPUKernelManager->LaunchKernel( this->m_ComputeUpdateGPUKernelHandle, ImageDim, globalSize, localSize );
}

} // end namespace itk

#endif

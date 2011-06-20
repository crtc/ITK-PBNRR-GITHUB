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
#ifndef __itkGPUDemonsRegistrationFunction_txx
#define __itkGPUDemonsRegistrationFunction_txx

#include "itkGPUDemonsRegistrationFunction.h"
#include "itkMacro.h"
#include "vnl/vnl_math.h"

namespace itk
{
/**
 * Default constructor
 */
template< class TFixedImage, class TMovingImage, class TDeformationField, class TParentFunction >
GPUDemonsRegistrationFunction< TFixedImage, TMovingImage, TDeformationField, TParentFunction >
::GPUDemonsRegistrationFunction()
{

  /*** Prepare GPU opencl program ***/

  std::ostringstream defines;

  if(TDeformationField::ImageDimension > 3 || TDeformationField::ImageDimension < 1)
  {
    itkExceptionMacro("GPUDenseFiniteDifferenceImageFilter supports 1/2/3D image.");
  }

  defines << "#define DIM_" << TDeformationField::ImageDimension << "\n";

  defines << "#define IMGPIXELTYPE ";
  GetTypenameInString( typeid ( typename TFixedImage::PixelType ), defines );

  defines << "#define BUFPIXELTYPE ";
  GetTypenameInString( typeid ( typename TDeformationField::PixelType::ValueType ), defines );

  defines << "#define OUTPIXELTYPE ";
  GetTypenameInString( typeid ( typename TDeformationField::PixelType::ValueType ), defines );

  std::string oclSrcPath = "./../OpenCL/GPUDemonsRegistrationFunction.cl";

  std::cout << "Defines: " << defines.str() << "Source code path: " << oclSrcPath << std::endl;

  // load and build program
  this->m_GPUKernelManager->LoadProgramFromFile( oclSrcPath.c_str(), defines.str().c_str() );

  // create kernel
  m_ComputeUpdateGPUKernelHandle = this->m_GPUKernelManager->CreateKernel("ComputeUpdate");

}

/**
 * Standard "PrintSelf" method.
 */
template< class TFixedImage, class TMovingImage, class TDeformationField, class TParentFunction  >
void
GPUDemonsRegistrationFunction< TFixedImage, TMovingImage, TDeformationField, TParentFunction >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

}

/**
 * Compute update at a specify neighbourhood
 */

template< class TFixedImage, class TMovingImage, class TDeformationField, class TParentFunction >
void
GPUDemonsRegistrationFunction< TFixedImage, TMovingImage, TDeformationField, TParentFunction >
::GPUComputeUpdate( DeformationFieldTypePointer output,
                    DeformationFieldTypePointer update,
                    void *gd )
{
  TFixedImage::ConstPointer  fixedImage  = dynamic_cast< const TFixedImage * >( this->GetFixedImage() );
  TMovingImage::ConstPointer movingImage = dynamic_cast< const TMovingImage * >( this->GetMovingImage() );
  typename DeformationFieldType::SizeType outSize = output->GetLargestPossibleRegion().GetSize();

  int imgSize[3];
  imgSize[0] = imgSize[1] = imgSize[2] = 1;

  int ImageDim = (int)DeformationFieldType::ImageDimension;

  for(int i=0; i<ImageDim; i++)
  {
    imgSize[i] = outSize[i];
  }

  size_t localSize[3], globalSize[3];
  localSize[0] = localSize[1] = localSize[2] = BLOCK_SIZE[ImageDim-1];
  for(int i=0; i<ImageDim; i++)
  {
    globalSize[i] = localSize[i]*(unsigned int)ceil((float)outSize[i]/(float)localSize[i]); // total # of threads
  }

  float normalizer = 1;

  // arguments set up
  int argidx = 0;
  this->m_GPUKernelManager->SetKernelArgWithImage(m_ComputeUpdateGPUKernelHandle, argidx++, fixedImage->GetGPUDataManager());
  this->m_GPUKernelManager->SetKernelArgWithImage(m_ComputeUpdateGPUKernelHandle, argidx++, movingImage->GetGPUDataManager());
  this->m_GPUKernelManager->SetKernelArgWithImage(m_ComputeUpdateGPUKernelHandle, argidx++, output->GetGPUDataManager());
  this->m_GPUKernelManager->SetKernelArgWithImage(m_ComputeUpdateGPUKernelHandle, argidx++, update->GetGPUDataManager());

  this->m_GPUKernelManager->SetKernelArg(m_ComputeUpdateGPUKernelHandle, argidx++, sizeof(float), &(normalizer));
  for(int i=0; i<ImageDim; i++)
  {
    this->m_GPUKernelManager->SetKernelArg(m_ComputeUpdateGPUKernelHandle, argidx++, sizeof(int), &(imgSize[i]));
  }

  // launch kernel
  this->m_GPUKernelManager->LaunchKernel(m_ComputeUpdateGPUKernelHandle, (int)DeformationFieldType::ImageDimension, globalSize, localSize );

}

} // end namespace itk

#endif

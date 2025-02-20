#ifndef __itkGPUImage_txx
#define __itkGPUImage_txx

#include "itkGPUImage.h"

namespace itk
{
//
// Constructor
//
template <class TPixel, unsigned int VImageDimension>
GPUImage< TPixel, VImageDimension >::GPUImage()
{
  m_DataManager = GPUImageDataManager< GPUImage< TPixel, VImageDimension > >::New();
  m_DataManager->SetTimeStamp( this->GetTimeStamp() );
}

template <class TPixel, unsigned int VImageDimension>
GPUImage< TPixel, VImageDimension >::~GPUImage()
{

}

template <class TPixel, unsigned int VImageDimension>
void GPUImage< TPixel, VImageDimension >::Allocate()
{
  // allocate CPU memory - calling Allocate() in superclass
  Superclass::Allocate();

  // allocate GPU memory
  this->ComputeOffsetTable();
  unsigned long numPixel = this->GetOffsetTable()[VImageDimension];
  m_DataManager->SetBufferSize( sizeof(TPixel)*numPixel );
  m_DataManager->SetImagePointer( this );
  m_DataManager->SetCPUBufferPointer( Superclass::GetBufferPointer() );
  m_DataManager->Allocate();

  /* prevent unnecessary copy from CPU to GPU at the beginning */
  m_DataManager->SetTimeStamp( this->GetTimeStamp() );
}

template< class TPixel, unsigned int VImageDimension >
void GPUImage< TPixel, VImageDimension >::Initialize()
{
  // CPU image initialize
  Superclass::Initialize();

  // GPU image initialize
  m_DataManager->Initialize();
  this->ComputeOffsetTable();
  unsigned long numPixel = this->GetOffsetTable()[VImageDimension];
  m_DataManager->SetBufferSize( sizeof(TPixel)*numPixel );
  m_DataManager->SetImagePointer( this );
  m_DataManager->SetCPUBufferPointer( Superclass::GetBufferPointer() );
  m_DataManager->Allocate();

  /* prevent unnecessary copy from CPU to GPU at the beginning */
  m_DataManager->SetTimeStamp( this->GetTimeStamp() );
}

template <class TPixel, unsigned int VImageDimension>
void GPUImage< TPixel, VImageDimension >::FillBuffer(const TPixel & value)
{
  m_DataManager->SetGPUBufferDirty();
  Superclass::FillBuffer( value );
}

template <class TPixel, unsigned int VImageDimension>
void GPUImage< TPixel, VImageDimension >::SetPixel(const IndexType & index, const TPixel & value)
{
  m_DataManager->SetGPUBufferDirty();
  Superclass::SetPixel( index, value );
}

template <class TPixel, unsigned int VImageDimension>
const TPixel & GPUImage< TPixel, VImageDimension >::GetPixel(const IndexType & index) const
{
  m_DataManager->UpdateCPUBuffer();
  return Superclass::GetPixel( index );
}

template <class TPixel, unsigned int VImageDimension>
TPixel & GPUImage< TPixel, VImageDimension >::GetPixel(const IndexType & index)
{
  /* Original version - very conservative
  m_DataManager->SetGPUBufferDirty();
  return Superclass::GetPixel( index );
  */
  m_DataManager->UpdateCPUBuffer();
  return Superclass::GetPixel( index );
}

template <class TPixel, unsigned int VImageDimension>
TPixel & GPUImage< TPixel, VImageDimension >::operator[] (const IndexType &index)
  {
  /* Original version - very conservative
  m_DataManager->SetGPUBufferDirty();
  return Superclass::operator[]( index );
  */
  m_DataManager->UpdateCPUBuffer();
  return Superclass::operator[] ( index );
  }

template <class TPixel, unsigned int VImageDimension>
const TPixel & GPUImage< TPixel, VImageDimension >::operator[] (const IndexType &index) const
  {
  m_DataManager->UpdateCPUBuffer();
  return Superclass::operator[] ( index );
  }

template <class TPixel, unsigned int VImageDimension>
void GPUImage< TPixel, VImageDimension >::SetPixelContainer(PixelContainer *container)
{
  Superclass::SetPixelContainer( container );
  m_DataManager->SetCPUDirtyFlag( false );
  m_DataManager->SetGPUDirtyFlag( true );
}

template <class TPixel, unsigned int VImageDimension>
void GPUImage< TPixel, VImageDimension >::UpdateBuffers()
{
  m_DataManager->UpdateCPUBuffer();
  m_DataManager->UpdateGPUBuffer();
}

template <class TPixel, unsigned int VImageDimension>
TPixel* GPUImage< TPixel, VImageDimension >::GetBufferPointer()
{
  /* Original version - very conservative
   * Always set GPU dirty (even though pixel values are not modified)
  m_DataManager->SetGPUBufferDirty();
  return Superclass::GetBufferPointer();
  */

  /* less conservative version - if you modify pixel value using
   * this pointer then you must set the image as modified manually!!! */
  m_DataManager->UpdateCPUBuffer();
  return Superclass::GetBufferPointer();
}

template <class TPixel, unsigned int VImageDimension>
const TPixel * GPUImage< TPixel, VImageDimension >::GetBufferPointer() const
{
  // const does not change buffer, but if CPU is dirty then make it up-to-date
  m_DataManager->UpdateCPUBuffer();
  return Superclass::GetBufferPointer();
}

template <class TPixel, unsigned int VImageDimension>
GPUDataManager::Pointer
GPUImage< TPixel, VImageDimension >::GetGPUDataManager() const
{
  typedef typename GPUImageDataManager< GPUImage >::Superclass GPUImageDataSuperclass;
  typedef typename GPUImageDataSuperclass::Pointer             GPUImageDataSuperclassPointer;

  return static_cast< GPUImageDataSuperclassPointer >( m_DataManager.GetPointer() );

  //return m_DataManager.GetPointer();
}

template <class TPixel, unsigned int VImageDimension>
void
GPUImage< TPixel, VImageDimension >::Graft(const DataObject *data)
{
  typedef GPUImageDataManager< GPUImage >              GPUImageDataManagerType;
  typedef typename GPUImageDataManagerType::Superclass GPUImageDataSuperclass;
  typedef typename GPUImageDataSuperclass::Pointer     GPUImageDataSuperclassPointer;

  // call the superclass' implementation
  Superclass::Graft(data);

  // Pass regular pointer to Graft() instead of smart pointer due to type
  // casting problem
  GPUImageDataManagerType* ptr = dynamic_cast<GPUImageDataManagerType*>(
      ( ( (GPUImage*)data)->GetGPUDataManager() ).GetPointer() );

  // Debug
  //std::cout << "GPU timestamp : " << m_DataManager->GetMTime() << ", CPU
  // timestamp : " << this->GetMTime() << std::endl;

  // call GPU data graft function
  m_DataManager->SetImagePointer( this );
  m_DataManager->Graft( ptr );

  // Synchronize timestamp of GPUImage and GPUDataManager
  m_DataManager->SetTimeStamp( this->GetTimeStamp() );

  // Debug
  //std::cout << "GPU timestamp : " << m_DataManager->GetMTime() << ", CPU
  // timestamp : " << this->GetMTime() << std::endl;

}

} // namespace itk

#endif

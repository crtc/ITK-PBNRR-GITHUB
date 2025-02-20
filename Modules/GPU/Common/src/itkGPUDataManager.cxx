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

#include "itkGPUDataManager.h"
//#define VERBOSE

namespace itk
{
// constructor
GPUDataManager::GPUDataManager()
{
  m_ContextManager = GPUContextManager::GetInstance();
  m_GPUBuffer = NULL;

  this->Initialize();
}

GPUDataManager::~GPUDataManager()
{
  if( m_GPUBuffer )
    {
    clReleaseMemObject(m_GPUBuffer);
    }
}

void GPUDataManager::SetBufferSize( unsigned int num )
{
  m_BufferSize = num;
}

void GPUDataManager::SetBufferFlag( cl_mem_flags flags )
{
  m_MemFlags = flags;
}

void GPUDataManager::Allocate()
{
  cl_int errid;

  if( m_BufferSize > 0 )
    {
#ifdef VERBOSE
std::cout << "Create GPU buffer of size " << m_BufferSize << " Bytes" << std::endl;
#endif
    m_GPUBuffer = clCreateBuffer(m_ContextManager->GetCurrentContext(), m_MemFlags, m_BufferSize, NULL, &errid);
    OclCheckError(errid);
    }

  //m_IsGPUBufferDirty = true;

  //this->UpdateGPUBuffer();
}

void GPUDataManager::SetCPUBufferPointer( void* ptr )
{
  m_CPUBuffer = ptr;
}

void GPUDataManager::SetCPUDirtyFlag( bool isDirty )
{
  m_IsCPUBufferDirty = isDirty;
}

void GPUDataManager::SetGPUDirtyFlag( bool isDirty )
{
  m_IsGPUBufferDirty = isDirty;
}

void GPUDataManager::SetGPUBufferDirty()
{
  this->UpdateCPUBuffer();
  m_IsGPUBufferDirty = true;
}

void GPUDataManager::SetCPUBufferDirty()
{
  this->UpdateGPUBuffer();
  m_IsCPUBufferDirty = true;
}

void GPUDataManager::UpdateCPUBuffer()
{
  m_Mutex.Lock();

  if( m_IsCPUBufferDirty && m_GPUBuffer != NULL && m_CPUBuffer != NULL )
    {
    cl_int errid;
#ifdef VERBOSE
    std::cout << "GPU->CPU data copy" << std::endl;
#endif
    errid = clEnqueueReadBuffer(m_ContextManager->GetCommandQueue(
                                  m_CommandQueueId), m_GPUBuffer, CL_TRUE, 0, m_BufferSize, m_CPUBuffer, 0, NULL, NULL);
    OclCheckError(errid);

    m_IsCPUBufferDirty = false;
    }

  m_Mutex.Unlock();
}

void GPUDataManager::UpdateGPUBuffer()
{
  m_Mutex.Lock();

  if( m_IsGPUBufferDirty && m_CPUBuffer != NULL && m_GPUBuffer != NULL )
    {
    cl_int errid;
#ifdef VERBOSE
    std::cout << "CPU->GPU data copy" << std::endl;
#endif
    errid = clEnqueueWriteBuffer(m_ContextManager->GetCommandQueue(
                                   m_CommandQueueId), m_GPUBuffer, CL_TRUE, 0, m_BufferSize, m_CPUBuffer, 0, NULL, NULL);
    OclCheckError(errid);

    m_IsGPUBufferDirty = false;
    }

  m_Mutex.Unlock();
}

cl_mem* GPUDataManager::GetGPUBufferPointer()
{
  SetCPUBufferDirty();
  return &m_GPUBuffer;
}

void* GPUDataManager::GetCPUBufferPointer()
{
  SetGPUBufferDirty();
  return m_CPUBuffer;
}

bool GPUDataManager::Update()
{
  if( m_IsGPUBufferDirty && m_IsCPUBufferDirty )
    {
    itkExceptionMacro("Cannot make up-to-date buffer because both CPU and GPU buffers are dirty");
    return false;
    }

  this->UpdateGPUBuffer();
  this->UpdateCPUBuffer();

  m_IsGPUBufferDirty = m_IsCPUBufferDirty = false;

  return true;
}

/**
 * NOTE: each device has a command queue. Therefore, changing command queue
 *       means change a compute device.
 */
void GPUDataManager::SetCurrentCommandQueue( int queueid )
{
  if( queueid >= 0 && queueid < (int)m_ContextManager->GetNumberOfCommandQueues() )
    {
    this->UpdateCPUBuffer();

    // Assumption: different command queue is assigned to different device
    m_CommandQueueId = queueid;

    m_IsGPUBufferDirty = true;
    }
  else
    {
    itkWarningMacro("Not a valid command queue id");
    }
}

int GPUDataManager::GetCurrentCommandQueueID()
{
  return m_CommandQueueId;
}

void GPUDataManager::Graft(const GPUDataManager* data)
{
  if( data )
    {
    m_BufferSize = data->m_BufferSize;
    m_ContextManager = data->m_ContextManager;
    m_CommandQueueId = data->m_CommandQueueId;
    m_GPUBuffer = data->m_GPUBuffer;
    m_CPUBuffer = data->m_CPUBuffer;
//    m_Platform  = data->m_Platform;
//    m_Context   = data->m_Context;
    m_IsCPUBufferDirty = data->m_IsCPUBufferDirty;
    m_IsGPUBufferDirty = data->m_IsGPUBufferDirty;
    }
}

void GPUDataManager::Initialize()
{
  if( m_ContextManager->GetNumberOfCommandQueues() > 0 )
    {
    m_CommandQueueId = 0; // default command queue
    }

  if( m_GPUBuffer ) // Release GPU memory if exists
    {
    clReleaseMemObject(m_GPUBuffer);
    }

  m_BufferSize = 0;
  m_GPUBuffer = NULL;
  m_CPUBuffer = NULL;
  m_MemFlags  = CL_MEM_READ_WRITE; // default flag
  m_IsGPUBufferDirty = false;
  m_IsCPUBufferDirty = false;
}

} // namespace itk

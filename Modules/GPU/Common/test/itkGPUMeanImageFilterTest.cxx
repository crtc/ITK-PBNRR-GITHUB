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

/**
 * Test program for itkGPUMeanImageFilter class
 *
 * This program creates a GPU Mean filter test pipelining.
 */

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMeanImageFilter.h"

#include "itkGPUImage.h"
#include "itkGPUKernelManager.h"
#include "itkGPUContextManager.h"
#include "itkGPUImageToImageFilter.h"
#include "itkGPUMeanImageFilter.h"

#include "itkRescaleIntensityImageFilter.h"
#include "itkTimeProbe.h"

/**
 * Testing GPU Mean Image Filter
 */
int itkGPUMeanImageFilterTest(int argc, char *argv[])
{
  if(!IsGPUAvailable())
  {
    std::cerr << "OpenCL-enabled GPU is not present." << std::endl;
    return EXIT_FAILURE;
  }

  // register object factory for GPU image and filter
  //itk::ObjectFactoryBase::RegisterFactory( itk::GPUImageFactory::New() );
  //itk::ObjectFactoryBase::RegisterFactory( itk::GPUMeanImageFilterFactory::New() );

  typedef   unsigned char  InputPixelType;
  typedef   unsigned char  OutputPixelType;

  //typedef itk::Image< InputPixelType,  3 >   InputImageType;
  //typedef itk::Image< OutputPixelType, 3 >   OutputImageType;

  typedef itk::GPUImage< InputPixelType,  3 >   InputImageType;
  typedef itk::GPUImage< OutputPixelType, 3 >   OutputImageType;

  typedef itk::ImageFileReader< InputImageType  >  ReaderType;
  typedef itk::ImageFileWriter< OutputImageType >  WriterType;

  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  if( argc <  3 )
  {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "inputfile outputfile " << std::endl;
    return EXIT_FAILURE;
    //reader->SetFileName( "/Users/wkjeong/Proj/ITK/Examples/Data/BrainProtonDensitySlice.png" ); //"C:/Users/wkjeong/Proj/ITK/Modules/GPU/Common/data/input-testvolume.nrrd" );
  }
  else
  {
    reader->SetFileName( argv[1] );
    writer->SetFileName( argv[2] );
  }

  //
  // Note: We use regular itk filter type here but factory will automatically create
  //       GPU filter for Median filter and CPU filter for threshold filter.
  //
  typedef itk::MeanImageFilter< InputImageType, OutputImageType > MeanFilterType;
  typedef itk::GPUMeanImageFilter< InputImageType, OutputImageType > GPUMeanFilterType;

  // Mean filter kernel radius
  InputImageType::SizeType indexRadius;
  indexRadius[0] = 2; // radius along x
  indexRadius[1] = 2; // radius along y
  indexRadius[2] = 2; // radius along z

  // test 1~8 threads for CPU
  for(int nThreads = 1; nThreads <= 8; nThreads++)
  {
    MeanFilterType::Pointer CPUFilter = MeanFilterType::New();

    itk::TimeProbe cputimer;
    cputimer.Start();

    CPUFilter->SetNumberOfThreads( nThreads );

    CPUFilter->SetInput( reader->GetOutput() );
    CPUFilter->SetRadius( indexRadius );
    CPUFilter->Update();

    cputimer.Stop();

    std::cout << "CPU mean filter took " << cputimer.GetMeanTime() << " seconds with "
              << CPUFilter->GetNumberOfThreads() << " threads.\n" << std::endl;

    // -------

    if( nThreads == 8 )
    {
      GPUMeanFilterType::Pointer GPUFilter = GPUMeanFilterType::New();

      itk::TimeProbe gputimer;
      gputimer.Start();

      GPUFilter->SetInput( reader->GetOutput() );
      GPUFilter->SetRadius( indexRadius );
      GPUFilter->Update();
      GPUFilter->GetOutput()->UpdateBuffers(); // synchronization point (GPU->CPU memcpy)

      gputimer.Stop();
      std::cout << "GPU mean filter took " << gputimer.GetMeanTime() << " seconds.\n" << std::endl;

      // ---------------
      // RMS Error check
      // ---------------

      double diff = 0;
      unsigned int nPix = 0;
      itk::ImageRegionIterator<OutputImageType> cit(CPUFilter->GetOutput(), CPUFilter->GetOutput()->GetLargestPossibleRegion());
      itk::ImageRegionIterator<OutputImageType> git(GPUFilter->GetOutput(), GPUFilter->GetOutput()->GetLargestPossibleRegion());

      for(cit.GoToBegin(), git.GoToBegin(); !cit.IsAtEnd(); ++cit, ++git)
      {
        //std::cout << "CPU : " << (double)(cit.Get()) << ", GPU : " << (double)(git.Get()) << std::endl;
        double err = (double)(cit.Get()) - (double)(git.Get());
        diff += err*err;
        nPix++;
      }

      std::cout << "RMS Error : " << sqrt( diff / (double)nPix ) << std::endl;
    }
  }

  return EXIT_SUCCESS;
}

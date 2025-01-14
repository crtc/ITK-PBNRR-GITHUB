#include "itkOclUtil.h"
#include <assert.h>
#include <iostream>

//
// Get the devices that are available.
//
cl_device_id* OclGetAvailableDevices(cl_platform_id platform, cl_device_type devType, cl_uint* numAvailableDevices)
{
  cl_device_id* availableDevices = NULL;
  cl_uint       totalNumDevices;

  // get total # of devices
  cl_int errid;

  errid = clGetDeviceIDs(platform, devType, 0, NULL, &totalNumDevices);
  OclCheckError( errid );

  cl_device_id* totalDevices = (cl_device_id *)malloc(totalNumDevices * sizeof(cl_device_id) );
  errid = clGetDeviceIDs(platform, devType, totalNumDevices, totalDevices, NULL);
  OclCheckError( errid );

  (*numAvailableDevices) = 0;

  // check available devices
  for(cl_uint i=0; i<totalNumDevices; i++)
    {
    cl_bool isAvailable;
    clGetDeviceInfo(totalDevices[i], CL_DEVICE_AVAILABLE, sizeof(cl_bool), &isAvailable, NULL);

    if(isAvailable)
      {
      (*numAvailableDevices)++;
      }
    }

  availableDevices = (cl_device_id *)malloc( (*numAvailableDevices) * sizeof(cl_device_id) );

  int idx = 0;
  for(cl_uint i=0; i<totalNumDevices; i++)
    {
    cl_bool isAvailable;
    clGetDeviceInfo(totalDevices[i], CL_DEVICE_AVAILABLE, sizeof(cl_bool), &isAvailable, NULL);

    if(isAvailable)
      {
      availableDevices[idx++] = totalDevices[i];
      }
    }

  free( totalDevices );

  return availableDevices;
}

//
// Get the device that has the maximum FLOPS in the current context
//
cl_device_id OclGetMaxFlopsDev(cl_context cxGPUContext)
{
  size_t        szParmDataBytes;
  cl_device_id* cdDevices;

  // get the list of GPU devices associated with context
  clGetContextInfo(cxGPUContext, CL_CONTEXT_DEVICES, 0, NULL, &szParmDataBytes);
  cdDevices = (cl_device_id*) malloc(szParmDataBytes);
  size_t device_count = szParmDataBytes / sizeof(cl_device_id);

  clGetContextInfo(cxGPUContext, CL_CONTEXT_DEVICES, szParmDataBytes, cdDevices, NULL);

  cl_device_id max_flops_device = cdDevices[0];
  int          max_flops = 0;

  size_t current_device = 0;

  // CL_DEVICE_MAX_COMPUTE_UNITS
  cl_uint compute_units;
  clGetDeviceInfo(cdDevices[current_device], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);

  // CL_DEVICE_MAX_CLOCK_FREQUENCY
  cl_uint clock_frequency;
  clGetDeviceInfo(cdDevices[current_device], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clock_frequency), &clock_frequency,
                  NULL);

  max_flops = compute_units * clock_frequency;
  ++current_device;

  while( current_device < device_count )
    {
    // CL_DEVICE_MAX_COMPUTE_UNITS
    //cl_uint compute_units;
    clGetDeviceInfo(cdDevices[current_device], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);

    // CL_DEVICE_MAX_CLOCK_FREQUENCY
    //cl_uint clock_frequency;
    clGetDeviceInfo(cdDevices[current_device], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clock_frequency), &clock_frequency,
                    NULL);

    int flops = compute_units * clock_frequency;
    if( flops > max_flops )
      {
      max_flops        = flops;
      max_flops_device = cdDevices[current_device];
      }
    ++current_device;
    }

  free(cdDevices);

  return max_flops_device;
}

//
// Print device name & info
//
void OclPrintDeviceName(cl_device_id device)
{
  cl_int err;

  char device_string[1024];

  err = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);
  printf("%s\n", device_string);

  size_t worksize[3];
  err = clGetDeviceInfo(device,CL_DEVICE_MAX_WORK_ITEM_SIZES,sizeof(worksize),&worksize,NULL);
  std::cout << "Maximum Work Item Sizes : { " << worksize[0] << ", " << worksize[1] << ", " << worksize[2] << " }" << std::endl;

  size_t maxWorkgroupSize;
  err = clGetDeviceInfo(device,CL_DEVICE_MAX_WORK_GROUP_SIZE,sizeof(maxWorkgroupSize),&maxWorkgroupSize,NULL);
  std::cout << "Maximum Work Group Size : " << maxWorkgroupSize << std::endl;
}

//
// Find the OpenCL platform that matches the "name"
//
cl_platform_id OclSelectPlatform(const char* name)
{
  char            chBuffer[1024];
  cl_uint         num_platforms;
  cl_platform_id* clPlatformIDs;
  cl_int          ciErrNum;
  cl_platform_id  clSelectedPlatformID = NULL;

  // Get OpenCL platform count
  ciErrNum = clGetPlatformIDs (0, NULL, &num_platforms);
  if (ciErrNum != CL_SUCCESS)
    {
    printf(" Error %i in clGetPlatformIDs Call !!!\n\n", ciErrNum);
    }
  else
    {
    if(num_platforms == 0)
      {
      printf("No OpenCL platform found!\n\n");
      }
    else
      {
      // if there's a platform or more, make space for ID's
      if ( (clPlatformIDs = (cl_platform_id*)malloc(num_platforms * sizeof(cl_platform_id) ) ) == NULL)
        {
        printf("Failed to allocate memory for cl_platform ID's!\n\n");
        }
      else
        {
        ciErrNum = clGetPlatformIDs (num_platforms, clPlatformIDs, NULL);
        if(ciErrNum == CL_SUCCESS)
          {
          clSelectedPlatformID = clPlatformIDs[0];         // default

  // debug
  ciErrNum = clGetPlatformInfo (clPlatformIDs[0], CL_PLATFORM_NAME, 1024, &chBuffer, NULL);
  std::cout << "Platform " << " : " << chBuffer << std::endl;
  //
          }

        if(num_platforms > 1)
          {
          std::cout << "Total # of platform : " << num_platforms << std::endl;

          for(cl_uint i = 0; i < num_platforms; ++i)
            {
            ciErrNum = clGetPlatformInfo (clPlatformIDs[i], CL_PLATFORM_NAME, 1024, &chBuffer, NULL);

// debug
            std::cout << "Platform " << i << " : " << chBuffer << std::endl;
//

            if(ciErrNum == CL_SUCCESS)
              {
              if(strstr(chBuffer, name) != NULL)
                {
                clSelectedPlatformID = clPlatformIDs[i];
                }
              }
            }
          }
        free(clPlatformIDs);
        }
      }
    }

  return clSelectedPlatformID;
}

void OclCheckError(cl_int error)
{
  static const char* errorString[] = {
    "CL_SUCCESS",
    "CL_DEVICE_NOT_FOUND",
    "CL_DEVICE_NOT_AVAILABLE",
    "CL_COMPILER_NOT_AVAILABLE",
    "CL_MEM_OBJECT_ALLOCATION_FAILURE",
    "CL_OUT_OF_RESOURCES",
    "CL_OUT_OF_HOST_MEMORY",
    "CL_PROFILING_INFO_NOT_AVAILABLE",
    "CL_MEM_COPY_OVERLAP",
    "CL_IMAGE_FORMAT_MISMATCH",
    "CL_IMAGE_FORMAT_NOT_SUPPORTED",
    "CL_BUILD_PROGRAM_FAILURE",
    "CL_MAP_FAILURE",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "CL_INVALID_VALUE",
    "CL_INVALID_DEVICE_TYPE",
    "CL_INVALID_PLATFORM",
    "CL_INVALID_DEVICE",
    "CL_INVALID_CONTEXT",
    "CL_INVALID_QUEUE_PROPERTIES",
    "CL_INVALID_COMMAND_QUEUE",
    "CL_INVALID_HOST_PTR",
    "CL_INVALID_MEM_OBJECT",
    "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
    "CL_INVALID_IMAGE_SIZE",
    "CL_INVALID_SAMPLER",
    "CL_INVALID_BINARY",
    "CL_INVALID_BUILD_OPTIONS",
    "CL_INVALID_PROGRAM",
    "CL_INVALID_PROGRAM_EXECUTABLE",
    "CL_INVALID_KERNEL_NAME",
    "CL_INVALID_KERNEL_DEFINITION",
    "CL_INVALID_KERNEL",
    "CL_INVALID_ARG_INDEX",
    "CL_INVALID_ARG_VALUE",
    "CL_INVALID_ARG_SIZE",
    "CL_INVALID_KERNEL_ARGS",
    "CL_INVALID_WORK_DIMENSION",
    "CL_INVALID_WORK_GROUP_SIZE",
    "CL_INVALID_WORK_ITEM_SIZE",
    "CL_INVALID_GLOBAL_OFFSET",
    "CL_INVALID_EVENT_WAIT_LIST",
    "CL_INVALID_EVENT",
    "CL_INVALID_OPERATION",
    "CL_INVALID_GL_OBJECT",
    "CL_INVALID_BUFFER_SIZE",
    "CL_INVALID_MIP_LEVEL",
    "CL_INVALID_GLOBAL_WORK_SIZE",
    };

  if(error != CL_SUCCESS)
    {
    // print error message
    const int errorCount = sizeof(errorString) / sizeof(errorString[0]);
    const int index = -error;

    if(index >= 0 && index < errorCount) printf("OpenCL Error : %s\n", errorString[index]);
    else printf("OpenCL Error : Unspecified Error\n");

    assert( false );
    }
}

/** Check if OpenCL-enabled GPU is present. */
bool IsGPUAvailable()
{
  cl_platform_id platformId = OclSelectPlatform("NVIDIA");

  if(platformId == NULL) return false;

  cl_device_type devType = CL_DEVICE_TYPE_GPU;

  // Get the devices
  cl_uint numDevices;
  OclGetAvailableDevices(platformId, devType, &numDevices);

  if(numDevices < 1) return false;

  return true;
}

void GetTypenameInString( const std::type_info& intype, std::ostringstream& ret )
{
  if ( intype == typeid ( unsigned char ) ||
       intype == typeid ( itk::Vector< unsigned char, 2 > ) ||
       intype == typeid ( itk::Vector< unsigned char, 3 > ) )
    {
    ret << "unsigned char\n";
    }
  else if ( intype == typeid ( char ) ||
            intype == typeid ( itk::Vector< char, 2 > ) ||
            intype == typeid ( itk::Vector< char, 3 > ) )
    {
    ret << "char\n";
    }
  else if ( intype == typeid ( short ) ||
            intype == typeid ( itk::Vector< short, 2 > ) ||
            intype == typeid ( itk::Vector< short, 3 > ) )
    {
    ret << "short\n";
    }
  else if ( intype == typeid ( int ) ||
            intype == typeid ( itk::Vector< int, 2 > ) ||
            intype == typeid ( itk::Vector< int, 3 > ) )
    {
    ret << "int\n";
    }
  else if ( intype == typeid ( unsigned int ) ||
            intype == typeid ( itk::Vector< unsigned int, 2 > ) ||
            intype == typeid ( itk::Vector< unsigned int, 3 > ) )
    {
    ret << "unsigned int\n";
    }
  else if ( intype == typeid ( float ) ||
            intype == typeid ( itk::Vector< float, 2 > ) ||
            intype == typeid ( itk::Vector< float, 3 > ) )
    {
    ret << "float\n";
    }
  else if ( intype == typeid ( double ) ||
            intype == typeid ( itk::Vector< double, 2 > ) ||
            intype == typeid ( itk::Vector< double, 3 > ) )
    {
    ret << "double\n";

    // enable 64bit computation
    ret << "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n";
    ret << "#pragma OPENCL EXTENSION cl_amd_fp64 : enable\n";
    }
  else
    {
    //std::cerr << "Pixeltype is not supported by GPUMeanImageFilter." <<
    // std::endl;
    itkGenericExceptionMacro("Pixeltype is not supported by the filter.");
    }
}

int GetPixelDimension( const std::type_info& intype )
{
  if ( intype == typeid ( unsigned char ) ||
       intype == typeid ( char ) ||
       intype == typeid ( short ) ||
       intype == typeid ( int ) ||
       intype == typeid ( unsigned int ) ||
       intype == typeid ( float ) ||
       intype == typeid ( double ) )
    {
    return 1;
    }
  else if( intype == typeid ( itk::Vector< unsigned char, 2 > ) ||
           intype == typeid ( itk::Vector< char, 2 > ) ||
           intype == typeid ( itk::Vector< short, 2 > ) ||
           intype == typeid ( itk::Vector< int, 2 > ) ||
           intype == typeid ( itk::Vector< unsigned int, 2 > ) ||
           intype == typeid ( itk::Vector< float, 2 > ) ||
           intype == typeid ( itk::Vector< double, 2 > ) )
    {
    return 2;
    }
  else if( intype == typeid ( itk::Vector< unsigned char, 3 > ) ||
           intype == typeid ( itk::Vector< char, 3 > ) ||
           intype == typeid ( itk::Vector< short, 3 > ) ||
           intype == typeid ( itk::Vector< int, 3 > ) ||
           intype == typeid ( itk::Vector< unsigned int, 3 > ) ||
           intype == typeid ( itk::Vector< float, 3 > ) ||
           intype == typeid ( itk::Vector< double, 3 > ) )
    {
    return 3;
    }
  else
    {
    itkGenericExceptionMacro("Pixeltype is not supported by the filter.");
    }
}

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
#ifndef __itkLabelMapContourOverlayImageFilter_h
#define __itkLabelMapContourOverlayImageFilter_h

#include "itkLabelMapFilter.h"
#include "itkBarrier.h"
#include "itkLabelOverlayFunctor.h"

namespace itk {

/** \class LabelMapContourOverlayImageFilter
 * \brief Apply a colormap to the contours (outlines) of each object in a label map
 *        and superimpose it on top of the feature image.
 *
 * The feature image is typically the image from which the labeling was
 * produced. Use the SetInput function to set the LabelMap, and the
 * SetFeatureImage function to set the feature image.
 *
 * Apply a colormap to a label map and put it on top of the input image.
 * The set of colors is a good selection of distinct colors. The opacity of
 * the label map can be defined by the user.
 * A background label produce a gray pixel with the same intensity
 * than the input one.
 *
 * \author Gaetan Lehmann. Biologie du Developpement et de la Reproduction, INRA de Jouy-en-Josas, France.
 *
 * This implementation was taken from the Insight Journal paper:
 * http://hdl.handle.net/1926/584  or
 * http://www.insight-journal.org/browse/publication/176
 *
 * \sa LabelMapToBinaryImageFilter, LabelMapToLabelImageFilter, LabelMapOverlayImageFilter
 * \ingroup ImageEnhancement  MathematicalMorphologyImageFilters
 * \ingroup ITK-Review
*/
template<class TLabelMap, class TFeatureImage, class TOutputImage=Image< RGBPixel< typename TFeatureImage::PixelType >, TFeatureImage::ImageDimension > >
class ITK_EXPORT LabelMapContourOverlayImageFilter :
    public LabelMapFilter<TLabelMap, TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef LabelMapContourOverlayImageFilter         Self;
  typedef LabelMapFilter<TLabelMap, TOutputImage>   Superclass;
  typedef SmartPointer<Self>                        Pointer;
  typedef SmartPointer<const Self>                  ConstPointer;

  /** Some convenient typedefs. */
  typedef TLabelMap                                LabelMapType;
  typedef typename LabelMapType::Pointer           LabelMapPointer;
  typedef typename LabelMapType::ConstPointer      LabelMapConstPointer;
  typedef typename LabelMapType::RegionType        LabelMapRegionType;
  typedef typename LabelMapType::PixelType         LabelMapPixelType;
  typedef typename LabelMapType::LabelObjectType   LabelObjectType;
  typedef typename LabelObjectType::LabelType      LabelType;

  typedef TFeatureImage                             FeatureImageType;
  typedef typename FeatureImageType::Pointer        FeatureImagePointer;
  typedef typename FeatureImageType::ConstPointer   FeatureImageConstPointer;
  typedef typename FeatureImageType::RegionType     FeatureImageRegionType;
  typedef typename FeatureImageType::PixelType      FeatureImagePixelType;

  typedef TOutputImage                             OutputImageType;
  typedef typename OutputImageType::Pointer        OutputImagePointer;
  typedef typename OutputImageType::ConstPointer   OutputImageConstPointer;
  typedef typename OutputImageType::RegionType     OutputImageRegionType;
  typedef typename OutputImageType::PixelType      OutputImagePixelType;
  typedef typename OutputImageType::IndexType      IndexType;
  typedef typename OutputImageType::SizeType       SizeType;
  typedef typename OutputImageType::RegionType     RegionType;

  typedef typename Functor::LabelOverlayFunctor<FeatureImagePixelType, LabelMapPixelType, OutputImagePixelType> FunctorType;

  /** ImageDimension constants */
  itkStaticConstMacro(LabelMapDimension, unsigned int,
                      TLabelMap::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
                      TOutputImage::ImageDimension);
  itkStaticConstMacro(ImageDimension, unsigned int,
                      TOutputImage::ImageDimension);

  enum
    {
    PLAIN = 0,
    CONTOUR = 1,
    SLICE_CONTOUR=2
    };

  enum
    {
    HIGH_LABEL_ON_TOP = 0,
    LOW_LABEL_ON_TOP = 1
    };

  /** Standard New method. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(LabelMapContourOverlayImageFilter,
               ImageToImageFilter);

   /** Set the feature image */
  void SetFeatureImage(TFeatureImage *input)
    {
    // Process object is not const-correct so the const casting is required.
    this->SetNthInput( 1, const_cast<TFeatureImage *>(input) );
    }

  /** Get the feature image */
  FeatureImageType * GetFeatureImage()
    {
    return static_cast<FeatureImageType*>(const_cast<DataObject *>(this->ProcessObject::GetInput(1)));
    }

   /** Set the input image */
  void SetInput1(TLabelMap *input)
    {
    this->SetInput( input );
    }

  /** Set the feature image */
  void SetInput2(TFeatureImage *input)
    {
    this->SetFeatureImage( input );
    }

  /** Set/Get the opacity of the colored label image. The value must be
   * between 0 and 1
   */
  itkSetMacro( Opacity, double );
  itkGetConstReferenceMacro( Opacity, double );

  /** Set/Get the overlay type - CONTOUR is used by default.
   */
  itkSetMacro( Type, int );
  itkGetConstReferenceMacro( Type, int );

  /** Set/Get the object priority - HIGH_LABEL_ON_TOP by default.
   */
  itkSetMacro( Priority, int );
  itkGetConstReferenceMacro( Priority, int );

  /** Set/Get the object dilation radius - 0 by default.
   */
  itkSetMacro( DilationRadius, SizeType );
  itkGetConstReferenceMacro( DilationRadius, SizeType );

  /** Set/Get the contour thickness - 1 by default.
   */
  itkSetMacro( ContourThickness, SizeType );
  itkGetConstReferenceMacro( ContourThickness, SizeType );

  /** Set/Get the slice dimension - defaults to image dimension - 1.
   */
  itkSetMacro( SliceDimension, int );
  itkGetConstReferenceMacro( SliceDimension, int );

protected:
  LabelMapContourOverlayImageFilter();
  ~LabelMapContourOverlayImageFilter() {};

  /** LabelMapContourOverlayImageFilter needs the entire input be
   * available. Thus, it needs to provide an implementation of
   * GenerateInputRequestedRegion(). */
  void GenerateInputRequestedRegion();

  /** LabelMapContourOverlayImageFilter will produce the entire output. */
  void EnlargeOutputRequestedRegion(DataObject *itkNotUsed(output));

  virtual void BeforeThreadedGenerateData();

  virtual void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType threadId );

  virtual void ThreadedProcessLabelObject( LabelObjectType * labelObject );

  void PrintSelf(std::ostream& os, Indent indent) const;

  virtual LabelMapType * GetLabelMap()
    {
    return m_TempImage;
    }

private:
  LabelMapContourOverlayImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  double                    m_Opacity;
  typename Barrier::Pointer m_Barrier;
  int                       m_Type;
  int                       m_Priority;
  SizeType                  m_ContourThickness;
  SizeType                  m_DilationRadius;
  int                       m_SliceDimension;

  LabelMapPointer           m_TempImage;

}; // end of class

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkLabelMapContourOverlayImageFilter.txx"
#endif

#endif

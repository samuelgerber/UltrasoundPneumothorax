
#include <tclap/CmdLine.h>
#include "ImageIO.h"

#include "itkImage.h"
#include "itkDerivativeImageFilter.h"
#include "itkDivideImageFilter.h"
#include "itkAbsImageFilter.h"
#include "itkAddImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkThresholdImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkRGBPixel.h"
#include "itkImageRegionIterator.h"
#include "itkMedianImageFilter.h"


int main(int argc, char **argv ){

  //Command line parsing
  TCLAP::CmdLine cmd("Derivative Ratio", ' ', "1");

  TCLAP::ValueArg<std::string> imageArg("v","volume","Ultrasound input volume", true, "",
      "filename");
  cmd.add(imageArg);

  TCLAP::ValueArg<std::string> prefixArg("p","prefix","Prefix for storing output images", true, "",
      "filename");
  cmd.add(prefixArg);

/*
  TCLAP::ValueArg<int> wArg("w","window","Window size", true, 50, "integer" );
  cmd.add(wArg);
*/

  try{
    cmd.parse( argc, argv );
  }
  catch (TCLAP::ArgException &e){
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return -1;
  }

  std::string prefix = prefixArg.getValue();

  typedef itk::Image< float, 2> ImageType;
  ImageType::Pointer image = ImageIO<ImageType>::ReadImage( imageArg.getValue() );
  ImageType::RegionType region = image->GetLargestPossibleRegion();
  ImageType::RegionType::SizeType size = region.GetSize();


  typedef itk::RescaleIntensityImageFilter< ImageType, ImageType> RescaleFilter;
  RescaleFilter::Pointer rescale = RescaleFilter::New();
  rescale->SetInput( image );
  rescale->SetOutputMaximum( 1.0 );
  rescale->SetOutputMinimum( 0 );
  rescale->Update();


  typedef itk::DerivativeImageFilter< ImageType, ImageType > DerivativeFilter;
  /*
  DerivativeFilter::Pointer derivativeX = DerivativeFilter::New();
  derivativeX->SetDirection(0);
  derivativeX->SetOrder( 2 );
  derivativeX->SetInput(image);

  DerivativeFilter::Pointer derivativeY = DerivativeFilter::New();
  derivativeY->SetDirection(1);
  derivativeY->SetOrder( 2 );
  derivativeY->SetInput(image);
*/

  DerivativeFilter::Pointer derivativeZ = DerivativeFilter::New();
  derivativeZ->SetDirection(0);
  derivativeZ->SetOrder( 2 );
  derivativeZ->SetInput( rescale->GetOutput() );


  typedef itk::AbsImageFilter<ImageType, ImageType> AbsFilter;
  /*
  AbsFilter::Pointer absX = AbsFilter::New();
  absX->SetInput( derivativeX->GetOutput() );

  AbsFilter::Pointer absY = AbsFilter::New();
  absY->SetInput( derivativeY->GetOutput() );
  */

  AbsFilter::Pointer absZ = AbsFilter::New();
  absZ->SetInput( derivativeZ->GetOutput() );

  /*
  typedef itk::MedianImageFilter<ImageType, ImageType> MedianFilter;
  MedianFilter::Pointer median = MedianFilter::New();
  median->SetInput( absY->GetOutput() );
  MedianFilter::InputSizeType radius;
  radius.Fill( 5 );
  median->SetRadius( radius );
  median->Update();
  ImageIO<ImageType>::saveImage( median->GetOutput(), "medY.nrrd");
*/

  /*
  typedef itk::AddImageFilter<ImageType, ImageType, ImageType> AddFilter;
  AddFilter::Pointer add = AddFilter::New();
  add->SetInput( absX->GetOutput() );
  add->SetConstant( 0.1 );
*/

  //yypedef itk::DiscreteGaussianImageFilter< ImageType, ImageType> GaussianFilter;
  typedef itk::SmoothingRecursiveGaussianImageFilter< ImageType, ImageType> GaussianFilter;

  /*
  GaussianFilter::Pointer gaussianX = GaussianFilter::New();
  gaussianX->SetInput( absX->GetOutput() );
  //gaussianX->SetUseImageSpacing(false);
  //gaussianX->SetVariance( 9 );
  gaussianX->SetSigma( 10 );

  GaussianFilter::Pointer gaussianY = GaussianFilter::New();
  gaussianY->SetInput( absY->GetOutput() );
 // gaussianY->SetUseImageSpacing(false);
  //gaussianY->SetVariance( 9 );
  gaussianY->SetSigma( 10 );
 */

  GaussianFilter::Pointer gaussianZ = GaussianFilter::New();
  gaussianZ->SetInput( absZ->GetOutput() );
 // gaussianY->SetUseImageSpacing(false);
  //gaussianY->SetVariance( 9 );
  GaussianFilter::SigmaArrayType sigma;
  sigma[0] = 16;
  sigma[1] = 5;
  gaussianZ->SetSigmaArray( sigma );


  /*
  typedef itk::DivideImageFilter<ImageType, ImageType, ImageType> DivideFilter;
  DivideFilter::Pointer divide = DivideFilter::New();
  divide->SetInput1( gaussianY->GetOutput() );
  divide->SetInput2( gaussianX->GetOutput() );
  divide->Update();

  ImageIO<ImageType>::saveImage( divide->GetOutput(), "derivative-ratio.nrrd");
*/


  typedef itk::ThresholdImageFilter<ImageType> ThresholdFilter;
  ThresholdFilter::Pointer threshold = ThresholdFilter::New();
  threshold->SetInput( gaussianZ->GetOutput() );
  threshold->ThresholdAbove( 0.05 );
  threshold->SetOutsideValue( 0.05 );
  threshold->Update();

  image = threshold->GetOutput();
  ImageType::IndexType index;
  index.Fill(0);
  image->SetPixel( index, 0.05 );

  ImageIO<ImageType>::saveImage( image, prefix + "_derivativeZ.nrrd");



  typedef itk::Image< unsigned char, 2> LabelImageType;
  typedef itk::RescaleIntensityImageFilter< ImageType, LabelImageType> LabelRescaleFilter;
  LabelRescaleFilter::Pointer lrescale = LabelRescaleFilter::New();
  lrescale->SetInput( image );
  lrescale->SetOutputMaximum( 4 );
  lrescale->SetOutputMinimum( 0 );
  LabelImageType::Pointer labelImage = lrescale->GetOutput();
  ImageIO< LabelImageType >::saveImage( labelImage, prefix + "_derivativeZ-label.nrrd");

  LabelRescaleFilter::Pointer lrescale2 = LabelRescaleFilter::New();
  lrescale2->SetInput( image );
  lrescale2->SetOutputMaximum( 255 );
  lrescale2->SetOutputMinimum( 0 );
  LabelImageType::Pointer labelImage2 = lrescale2->GetOutput();
  ImageIO< LabelImageType >::saveImage( labelImage2, prefix + "_derivativeZ.png");


  typedef itk::RescaleIntensityImageFilter< LabelImageType, LabelImageType> LabelRescaleFilter2;
  LabelRescaleFilter2::Pointer lrescale1 = LabelRescaleFilter2::New();
  lrescale1->SetInput( labelImage );
  lrescale1->SetOutputMaximum( 255 );
  lrescale1->SetOutputMinimum( 0 );
  LabelImageType::Pointer labelImage1 = lrescale1->GetOutput();
  ImageIO< LabelImageType >::saveImage( labelImage1, prefix + "_derivativeZ-label.png");

/*
  typedef itk::Image< unsigned char, 3> LabelImageType;
  typedef itk::BinaryThresholdImageFilter<ImageType, LabelImageType> BinaryThresholdFilter;
  BinaryThresholdFilter::Pointer bthreshold = BinaryThresholdFilter::New();
  bthreshold->SetInput( gaussianZ->GetOutput() );
  bthreshold->SetUpperThreshold( 4.5 );
  bthreshold->SetInsideValue( 0 );
  bthreshold->SetOutsideValue( 1 );
  bthreshold->Update();

  LabelImageType::Pointer labelImage = bthreshold->GetOutput();
  ImageIO<LabelImageType>::saveImage( labelImage, "derivativeZ-label.nrrd");

*/


  /*
  typedef itk::RGBPixel<unsigned char> RGBPixelType;
  typedef itk::Image<RGBPixelType> RGBImageType;
  typedef itk::CastImageFilter< ImageType, RGBImageType> RGBCastFilter;
  RGBCastFilter::Pointer rgbConvert = RGBCastFilter::New();
  rgbConvert->SetInput( image );
  rgbConvert->Update();
  RGBImageType::Pointer overlayImage = rgbConvert->GetOutput();
  itk::ImageRegionIterator<RGBImageType> overlayIterator( overlayImage, overlayImage->GetLargestPossibleRegion() );
  itk::ImageRegionIterator<LabelImageType> labelIterator( labelImage, labelImage->GetLargestPossibleRegion() );
  double alpha = 0.3;
  while( !overlayIterator.IsAtEnd() ){
    RGBImageType::PixelType pixel = overlayIterator.Get();
    double p = labelIterator.Get();
    if(p == 1){
      pixel[0] = alpha * 255 + (1-alpha) * pixel[0];
      pixel[1] = (1-alpha) * pixel[1];
      pixel[2] = (1-alpha) * pixel[2];
    }
    overlayIterator.Set( pixel );

    ++labelIterator;
    ++overlayIterator;
  }
  ImageIO<RGBImageType>::saveImage( overlayImage, "derivative-ratio-label.nrrd");
  */

  return EXIT_SUCCESS;
}


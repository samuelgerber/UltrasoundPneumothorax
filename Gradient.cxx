
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
#include "itkCastImageFilter.h"
#include "itkRGBPixel.h"
#include "itkImageRegionIterator.h"
#include "itkMedianImageFilter.h"

#include <itkGradientImageFilter.h>

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
  const int windowSize = 50;

  typedef itk::Image< float, 3> ImageType;
  ImageType::Pointer image = ImageIO<ImageType>::ReadImage( imageArg.getValue() );
  ImageType::RegionType region = image->GetLargestPossibleRegion();
  ImageType::RegionType::SizeType size = region.GetSize();

  typedef itk::GradientImageFilter< ImageType > GradientFilter;
  GradientFilter::Pointer grad = GradientFilter::New();
  grad->SetInput(image);
  grad->Update();

  typedef GradientFilter::OutputImageType GradImage;
  GradImage::Pointer gImage = grad->GetOutput();




  ImageIO<LabelImageType>::saveImage( labelImage, "derivativeX-label.nrrd");

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


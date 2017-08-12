
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
#include "vnl_fft_1d.h"

#include <sstream>

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
  const int fftSize = 127;

  typedef itk::Image< float, 3> FloatImage;
  FloatImage::Pointer image = ImageIO< FloatImage >::ReadImage( imageArg.getValue() );
  FloatImage::RegionType region = image->GetLargestPossibleRegion();
  FloatImage::SpacingType spacing = image->GetSpacing();
  FloatImage::RegionType::SizeType size = region.GetSize();


  FloatImage::RegionType spectraRegion = region;
  FloatImage::SpacingType spectraSpacing = spacing;

  FloatImage::Pointer  spectra = FloatImage::New();
  spectra->SetRegions( spectraRegion );
  spectra->SetSpacing( spectraSpacing );
  spectra->Allocate();

  typedef vcl_complex< float >      ComplexType;
  typedef vnl_vector< ComplexType > ComplexVectorType;
  typedef std::vector< float >      SpectraVectorType;
  typedef vnl_fft_1d< float >       FFT1DType;
  ComplexVectorType complexVector( size[2] );
  FloatImage::IndexType index;
  for(int i=0; i < size[0]; i++){
    index[0] = i;
    for(int j = 0; j < size[1]; j++){
      index[1] = j;
      for(int k=fftSize-1; k < size[2]; k++){
        for(int l = 0; l < fftSize; l++){
          index[2] = l;
          complexVector[l] = image->GetPixel(index);
        }
        FFT1DType fft1D( fftSize );
        fft1D.bwd_transform( complexVector );
        for(int l=0; l < size[2]; l++){
          index[2] = l;
          spectra->SetPixel( index, std::real( complexVector[l] * std::conj( complexVector[l] ) ) );
        }
      }
    }
    std::cout << i << std::endl;
  }
  std::cout << std::endl;

  std::stringstream ss;
  ss << "spectra1D.nrrd";
  ImageIO< FloatImage >::saveImage( spectra, ss.str() );

  return EXIT_SUCCESS;
}


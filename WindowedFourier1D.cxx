
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
  const int fftSize = 64;

  typedef itk::Image< float, 3> FloatImage;
  FloatImage::Pointer image = ImageIO< FloatImage >::ReadImage( imageArg.getValue() );
  FloatImage::RegionType region = image->GetLargestPossibleRegion();
  FloatImage::SpacingType spacing = image->GetSpacing();
  FloatImage::RegionType::SizeType size = region.GetSize();


  const int stepX = 2;
  const int stepY = 2;
  const int stepZ = fftSize / 2;
  FloatImage::RegionType spectraRegion = region;
  FloatImage::SizeType spectraSize = size;
  spectraSize[0] /= stepX;
  spectraSize[1] /= stepY;
  spectraSize[2] /= stepZ;
  spectraRegion.SetSize( spectraSize );
  FloatImage::SpacingType spectraSpacing = spacing;
  spectraSpacing[0] *= ( (float) size[0] ) / (spectraSize[0]) ;
  spectraSpacing[1] *= ( (float) size[1] ) / (spectraSize[1]) ;
  spectraSpacing[2] *= ( (float) size[2] ) / (spectraSize[2]) ;

  std::vector< FloatImage::Pointer > spectra( fftSize );
  for(int i=0; i<spectra.size(); i++){
    spectra[i] = FloatImage::New();
    spectra[i]->SetRegions( spectraRegion );
    spectra[i]->SetSpacing( spectraSpacing );
    spectra[i]->Allocate();
  }

  typedef vcl_complex< float >      ComplexType;
  typedef vnl_vector< ComplexType > ComplexVectorType;
  typedef std::vector< float >      SpectraVectorType;
  typedef vnl_fft_1d< float >       FFT1DType;
  ComplexVectorType complexVector(fftSize);
  FloatImage::IndexType index;
  FloatImage::IndexType spectraIndex;
  for(int i=0; i < size[0]; i+=stepX){
    index[0] = i;
    spectraIndex[0] = i /stepX;
    for(int j = 0; j < size[1]; j+=stepY){
      index[1] = j;
      spectraIndex[1] = j /stepY;
      for(int k=fftSize-1; k < size[2]; k += stepZ){
        for(int l = 0; l < fftSize; l++){
          index[2] = k-l;
          complexVector[l] = image->GetPixel(index);
        }
        FFT1DType fft1D( fftSize );
        fft1D.bwd_transform( complexVector );
        spectraIndex[2] = k /stepZ;
        for(int l=0; l<fftSize; l++){
          spectra[l]->SetPixel( spectraIndex, std::real( complexVector[l] * std::conj( complexVector[l] ) ) );
        }
      }
    }
    std::cout << i << std::endl;
  }
  std::cout << std::endl;

  for(int i =0; i<spectra.size(); i++){
    std::stringstream ss;
    ss << "spectra_" << i << ".nrrd";
    ImageIO< FloatImage >::saveImage( spectra[i], ss.str());
  }

  return EXIT_SUCCESS;
}


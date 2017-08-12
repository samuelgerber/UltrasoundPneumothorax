
#include <sstream>

#include <tclap/CmdLine.h>

#include "ImageIO.h"


#include "itkImage.h"

#include <itkWrapPadImageFilter.h>
#include "itkForwardFFTImageFilter.h"
#include "itkInverseFFTImageFilter.h"
#include "itkComplexToModulusImageFilter.h"

#include "itkSimoncelliIsotropicWavelet.h"
#include "itkWaveletFrequencyFilterBankGenerator.h"
#include "itkWaveletFrequencyForward.h"

int main(int argc, char **argv ){

  //Command line parsing
  TCLAP::CmdLine cmd("Perform 1D WindowedCrossCorrelation", ' ', "1");

  TCLAP::ValueArg<std::string> imageArg("v","volume","Ultrasound input volume", true, "",
      "filename");
  cmd.add(imageArg);

  TCLAP::ValueArg<std::string> prefixArg("p","prefix","Prefix for storing output images", true, "",
      "filename");
  cmd.add(prefixArg);

  /*
  TCLAP::ValueArg<int> dirArg("d","direction","Direction of crosscorrlation", true, 2, "integer" );
  cmd.add(dirArg);

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
  int nLevels = 4;

  typedef itk::Image< float, 3> ImageType;
  ImageType::Pointer image = ImageIO<ImageType>::ReadImage( imageArg.getValue() );

  typedef itk::WrapPadImageFilter< ImageType, ImageType > PadFilterType;
  PadFilterType::Pointer padFilter = PadFilterType::New();
  padFilter->SetInput( image );
  PadFilterType::SizeType padding;
  padding[0] = 224;
  padding[1] = 460;
  padding[2] = 76;
  padFilter->SetPadUpperBound( padding );
  padFilter->Update();

  typedef itk::ForwardFFTImageFilter<ImageType> FFTType;
  FFTType::Pointer fftFilter = FFTType::New();
  fftFilter->SetInput( padFilter->GetOutput()  );
  fftFilter->Update();

  typedef FFTType::OutputImageType ComplexImageType;
  typedef itk::Point<double, 3> PointType;
  typedef itk::SimoncelliIsotropicWavelet<double, 3, PointType> SimoncelliWaveletType;
  typedef itk::WaveletFrequencyFilterBankGenerator< ComplexImageType, SimoncelliWaveletType> WaveletFilterBankType;
  typedef itk::WaveletFrequencyForward<ComplexImageType, ComplexImageType, WaveletFilterBankType> WaveletForwardType;
  WaveletForwardType::Pointer waveletFilter = WaveletForwardType::New();
  waveletFilter->SetInput( fftFilter->GetOutput() );
  waveletFilter->SetLevels( nLevels );
  waveletFilter->SetHighPassSubBands( 1 );
  waveletFilter->Update();

  ComplexImageType::SizeType size = fftFilter->GetOutput()->GetLargestPossibleRegion().GetSize();
  std:: cout << WaveletForwardType::ComputeMaxNumberOfLevels( size ) << std::endl;
  std:: cout << size << std::endl;

  //typedef itk::ComplexToModulusImageFilter<ComplexImageType, ImageType> ModulusFilterType;
  //ModulusFilterType::Pointer modulusFilter = ModulusFilterType::New();

  typedef itk::InverseFFTImageFilter<ComplexImageType, ImageType> IFFTType;
  IFFTType::Pointer ifftFilter = IFFTType::New();

  for(int i=0; i<nLevels; i++){
    //std::cout << waveletFilter->GetOutputsHighPassByLevel(i).size() << std::endl;
    ifftFilter->SetInput( waveletFilter->GetOutput(i) );
    ifftFilter->Update();

    std::stringstream ss;
    ss << "wavelet_" << i << ".nrrd";
    ImageIO<ImageType>::saveImage( ifftFilter->GetOutput(), ss.str() );
  }

  return EXIT_SUCCESS;
}

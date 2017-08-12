
#include <tclap/CmdLine.h>
#include "itkImage.h"
#include "ImageIO.h"

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
  const int windowSize = 50;

  typedef itk::Image< float, 3> ImageType;
  ImageType::Pointer image = ImageIO<ImageType>::ReadImage( imageArg.getValue() );
  ImageType::RegionType region = image->GetLargestPossibleRegion();
  ImageType::RegionType::SizeType size = region.GetSize();

  ImageType::Pointer corr = ImageType::New();
  corr->SetRegions(region);
  corr->Allocate();
  corr->FillBuffer( 0 );

  ImageType::IndexType index;
  for( int i = 0; i < size[0]; i++){
    index[0] = i;
    for( int j=0; j < size[1]; j++){
      index[1] = j;
      for( int k = windowSize-1; k < size[2]; k += windowSize / 2 ){

        float min = 1;
        float max = -1;
        float mean= 0;
        for(int kk = 0; kk < windowSize; kk++){
          index[2] = k - kk;
          float p1 = image->GetPixel(index) / 255.0;
          mean += p1;
          min = std::min(min, p1);
          max = std::max(max, p1);
        }
        mean /= windowSize;

        float var = 0;
        for(int kk = 0; kk < windowSize; kk++){
          index[2] = k - kk;
          float p1 = image->GetPixel(index) / 255.0 - mean;
          var += p1 * p1;
        }
        if( max == min){
          var = 0;
        }
        else{
          var /= (windowSize-1);
          var /= (max-min);
        }

        for(int kk = 0; kk < windowSize/2; kk++){
          index[2] = k - kk;
          corr->SetPixel(index, var );
        }
      }
    }
    std::cout << "i: " << i << " of " << size[0] << std::endl;
  }
  ImageIO<ImageType>::saveImage(corr, "var.nrrd");

  return EXIT_SUCCESS;
}

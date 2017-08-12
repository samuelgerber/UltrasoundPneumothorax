
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
  const int windowSize = 100;

  typedef itk::Image< float, 3> ImageType;
  ImageType::Pointer image = ImageIO<ImageType>::ReadImage( imageArg.getValue() );
  ImageType::RegionType region = image->GetLargestPossibleRegion();
  ImageType::RegionType::SizeType size = region.GetSize();

  ImageType::Pointer corr = ImageType::New();
  corr->SetRegions(region);
  corr->Allocate();
  corr->FillBuffer( 1 );

  ImageType::IndexType index;
  for( int i = 0; i < size[0]; i++){
    index[0] = i;
    for( int k = 10; k < size[1]; k++ ){
      for( int j = windowSize-1; j < size[2]; j += windowSize / 2 ){

        float sum1 = 0;
        float sum2 = 0;
        for(int jj = 0; jj < windowSize; jj++){
          index[2] = j-jj;
          index[1] = k;
          float p1 = image->GetPixel(index)/255.0;
          sum1 += p1;
          index[1] = k-10;
          float p2 = image->GetPixel(index)/255.0;
          sum2 += p2;
        }
        float mean1 = sum1 / windowSize;
        float mean2 = sum2 / windowSize;

        //std::cout << mean1 << std::endl;

        float ssum1 = 0;
        float ssum2 = 0;
        float r2 = 0;
        for(int jj = 0; jj < windowSize; jj++){
          index[2] = j-jj;
          index[1] = k;
          float p1 = image->GetPixel(index)/255.0 - mean1;
          ssum1 += p1*p1;
          index[1] = k-10;
          float p2 = image->GetPixel(index)/255.0 - mean2;
          ssum2 = p2*p2;

          r2 += p1*p2;
        }

        r2 /= sqrt( ssum1 ) * sqrt( ssum2 );
        if( r2 != r2 ){
           r2 = 1;
        }
        else{
          r2 = fabs( r2 );
          if( r2 > 1){
            //r2 = 1;
          }
        }

        index[1] = k-5;
        for(int jj = 0; jj < windowSize / 2; jj++){
          index[2] = j - jj;
          corr->SetPixel(index, r2 );
        }
      }
    }
    std::cout << "i: " << i << " of " << size[0] << std::endl;
  }
  ImageIO<ImageType>::saveImage(corr, "corr.nrrd");

  return EXIT_SUCCESS;
}

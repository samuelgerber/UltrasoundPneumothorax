CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(UltrasoundPneumothorax)

find_package( ITK COMPONENTS
  IsotropicWavelets
  ITKFFT 
  ITKCommon
  ITKIOImageBase
  ITKImageGrid
  ITKImageIntensity
  ITKStatistics
  ITKIONRRD
  ITKIOPNG
)
INCLUDE(${ITK_USE_FILE} )

ADD_EXECUTABLE( WindowedCrossCorrelation WindowedCrossCorrelation.cxx )
TARGET_LINK_LIBRARIES (WindowedCrossCorrelation ${ITK_LIBRARIES} )

ADD_EXECUTABLE( WindowedFourier1D WindowedFourier1D.cxx )
TARGET_LINK_LIBRARIES( WindowedFourier1D ${ITK_LIBRARIES} )

ADD_EXECUTABLE( Fourier1D Fourier1D.cxx )
TARGET_LINK_LIBRARIES( Fourier1D ${ITK_LIBRARIES} )

ADD_EXECUTABLE(TimeVariance TimeVariance.cxx )
TARGET_LINK_LIBRARIES(TimeVariance ${ITK_LIBRARIES} )

ADD_EXECUTABLE(PatchStatistics PatchStatistics.cxx )
TARGET_LINK_LIBRARIES (PatchStatistics ${ITK_LIBRARIES} )

ADD_EXECUTABLE(IsoWavelet IsoWavelet.cxx )
TARGET_LINK_LIBRARIES (IsoWavelet ${ITK_LIBRARIES} )

ADD_EXECUTABLE(DerivativeRatio DerivativeRatio.cxx )
TARGET_LINK_LIBRARIES (DerivativeRatio ${ITK_LIBRARIES} )

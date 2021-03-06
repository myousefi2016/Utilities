#include "itkImageFileReader.h"
#include "itkVectorImageFileReader.h"
#include "itkVector.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridWriter.h"
#include "vtkFloatArray.h"
#include "vtkPoints.h"
#include "vtkPointData.h"


#include "global.h"

int main( int argc, char *argv[] )
{
  if ( argc < 3 )
    {
    std::cout << "Usage: " << argv[0] << " inputDeformationField outputVTKFile [maskImage] [slice] [whichAxis]" << std::endl;
    exit( 1 );
    }

  typedef itk::Image<PixelType, ImageDimension> ImageType;
  typedef itk::Image<int, ImageDimension> MaskImageType;
  
  typedef double RealType;
  typedef itk::Vector<RealType, ImageDimension> VectorType;
  typedef itk::Image<VectorType, ImageDimension> DeformationFieldType;
  
  typedef itk::VectorImageFileReader<ImageType, DeformationFieldType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( argv[1] );
  reader->SetUseAvantsNamingConvention( true );
  reader->Update();
  
  MaskImageType::Pointer mask = MaskImageType::New();
  if ( argc >= 4 )
    {
    typedef itk::ImageFileReader<MaskImageType> MaskReaderType;
    MaskReaderType::Pointer maskreader = MaskReaderType::New();
    maskreader->SetFileName( argv[3] );
    maskreader->Update();
    mask = maskreader->GetOutput();
    }
  else
    {
    mask->SetOrigin( reader->GetOutput()->GetOrigin() );
    mask->SetSpacing( reader->GetOutput()->GetSpacing() );
    mask->SetRegions( reader->GetOutput()->GetLargestPossibleRegion() );
    mask->Allocate();
    mask->FillBuffer( 1 );
    }  

  double origin[ImageDimension];
  double spacing[ImageDimension];
  int    size[ImageDimension];
  int totalsize = 1;

  for ( unsigned int i = 0; i < ImageDimension; i++ )
    {
    origin[i] = reader->GetOutput()->GetOrigin()[i];
    spacing[i] = reader->GetOutput()->GetSpacing()[i];
    size[i] = reader->GetOutput()->GetLargestPossibleRegion().GetSize()[i];
    if ( static_cast<unsigned int>( argc ) > 4 && 
         atoi( argv[5] ) == static_cast<int>( i ) )
      {
      size[i] = 1;
      }              
    totalsize *= size[i];
    }

  int totalPoints = totalsize;

  vtkUnstructuredGrid *field = vtkUnstructuredGrid::New();
  vtkPoints *points = vtkPoints::New();
  points->Allocate( totalPoints );
  vtkFloatArray *vectors = vtkFloatArray::New();
  vectors->SetNumberOfComponents( 3 );
  vectors->SetNumberOfTuples( totalPoints );

  float x[3], v[3];
  int offset = 0;

  itk::ImageRegionIteratorWithIndex<MaskImageType> It
    ( mask, mask->GetLargestPossibleRegion() );
  for ( It.GoToBegin(); !It.IsAtEnd(); ++It )
    {
    DeformationFieldType::IndexType idx = It.GetIndex();
    
    if ( ( argc > 4 && idx[atoi( argv[5] )] != atoi( argv[4] ) ) || It.Get() == 0 )
      {
      continue;  
      }              
    DeformationFieldType::PointType point;
    reader->GetOutput()->TransformIndexToPhysicalPoint( idx, point ); 
 
    VectorType V = reader->GetOutput()->GetPixel( idx ); 
    for ( unsigned int i = 0; i < ImageDimension; i++ )
      {
      x[i] = point[i];
      v[i] = V[i]; 
      }
//    offset = idx[0] + idx[1]*(size[1]+1) + idx[2]*(size[1]+1)*(size[3]+1);       
    points->InsertPoint( offset, x );
    vectors->InsertTuple( offset++, v );
    }

  field->SetPoints( points );
  field->GetPointData()->SetVectors( vectors );

  points->Delete();
  vectors->Delete();

  vtkUnstructuredGridWriter *writer = vtkUnstructuredGridWriter::New();
  writer->SetInput( field );
//  writer->SetFileTypeToBinary();
  writer->SetFileName( argv[2] );
  writer->Write();
  return 0;
}

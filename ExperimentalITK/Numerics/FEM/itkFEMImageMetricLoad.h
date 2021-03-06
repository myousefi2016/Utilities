/*=========================================================================

Program:   Insight Segmentation & Registration Toolkit
Module:    $RCSfile: itkFEMImageMetricLoad.h,v $
Language:  C++

Date:      $Date: 2008/10/18 00:22:49 $
Version:   $Revision: 1.1.1.1 $

Copyright (c) Insight Software Consortium. All rights reser
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for detail.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _itkFEMImageMetricLoad_h_
#define _itkFEMImageMetricLoad_h_

#include "itkFEMLoadElementBase.h"

#include "itkImageToImageMetric.h"
#include "itkTranslationTransform.h"
#include "itkDeformationFieldTransform.h"
#include "itkNeighborhoodIterator.h"
#include "itkLinearInterpolateImageFunction.h"

namespace itk 
{
namespace fem
{

/**
 * \class ImageMetricLoad
 * \brief General image pair load that uses the itkImageToImageMetrics.
 *
 * LoadImageMetric computes FEM gravity loads by using derivatives provided 
 * by itkImageToImageMetrics (e.g. mean squares intensity difference.)
 * The function responsible for this is called Fg, as required by the FEMLoad
 * standards.  It takes a vnl_vector as input.
 * We assume the vector input is of size 2*ImageDimension.
 * The 0 to ImageDimension-1 elements contain the position, p,
 * in the reference (moving) image.  The next ImageDimension to 2*ImageDimension-1
 * elements contain the value of the vector field at that point, v(p).
 *
 * Then, we evaluate the derivative at the point p+v(p) with respect to
 * some region of the target (fixed) image by calling the metric with 
 * the translation parameters as provided by the vector field at p.
 * The metrics return both a scalar similarity value and vector-valued derivative.  
 * The derivative is what gives us the force to drive the FEM registration.
 * These values are computed with respect to some region in the Fixed image.
 * This region size may be set by the user by calling SetMetricRadius.
 * As the metric derivative computation evolves, performance should improve
 * and more functionality will be available (such as scale selection).
 */ 
template<class TMovingImage, class TFixedImage> 
class ImageMetricLoad
: public LoadElement
{
FEM_CLASS(ImageMetricLoad, LoadElement)

public:
  typedef TMovingImage                                    MovingImageType;
  typedef TFixedImage                                     FixedImageType;
  typedef typename MovingImageType::Pointer               MovingImageTypePointer;
  typedef typename FixedImageType::Pointer                FixedImageTypePointer;

  itkStaticConstMacro(ImageDimension, unsigned int,
                      MovingImageType::ImageDimension);

  typedef NeighborhoodIterator<MovingImageType>           NeighborhoodIteratorType; 
  typedef typename NeighborhoodIteratorType::RadiusType   RadiusType;

  typedef typename LoadElement::Float                     Float;
  typedef vnl_vector<Float>                               FEMVectorType;

  typedef double                                          RealType;
  typedef Image<RealType, 
          itkGetStaticConstMacro(ImageDimension)>         RealImageType;
  typedef Vector<RealType,
                 itkGetStaticConstMacro(ImageDimension)>  VectorType;
  typedef Image<VectorType, 
                itkGetStaticConstMacro(ImageDimension)>   DeformationFieldType;
  typedef typename DeformationFieldType::Pointer          DeformationFieldTypePointer;

  /** Typedef support for the image to image metric */
  typedef ImageToImageMetric<FixedImageType,
                             MovingImageType>             MetricBaseType;
  typedef typename MetricBaseType::Pointer                MetricBaseTypePointer;

  /** Typedef support for the transform */
  typedef DeformationFieldTransform<DeformationFieldType> DeformationFieldTransformType;
  typedef typename DeformationFieldTransformType::Pointer DeformationFieldTransformTypePointer;

//  typedef TranslationTransform<RealType,  
//                  itkGetStaticConstMacro(ImageDimension)> DeformationFieldTransformType;
//  typedef typename DeformationFieldTransformType::Pointer DeformationFieldTransformTypePointer;

  /** Typedef support for the interpolation function */
  typedef InterpolateImageFunction<MovingImageType, 
                                   RealType>              ImageInterpolatorType;
  typedef typename ImageInterpolatorType::Pointer         ImageInterpolatorTypePointer; 


  /** Main functions */

  ImageMetricLoad(); 
  ~ImageMetricLoad() {} 

  void InitializeMetric(void);
  FEMVectorType Fe(FEMVectorType, FEMVectorType);
  RealType EvaluateMetricGivenSolution(Element::ArrayType* el, RealType step = 1.0);

  static Baseclass* NewImageMetricLoad(void)
    { 
      return new ImageMetricLoad; 
    }

  /** Set/Get functions */
  void SetMetric(MetricBaseTypePointer M) 
    { 
      m_Metric = M; 
    }
  MetricBaseTypePointer GetMetric() 
    { 
      return m_Metric; 
    }  
  RealType GetMetricValue(FEMVectorType);    
  void SetMovingImage(MovingImageTypePointer I)
    { 
      m_MovingImage = I; 
    }
  void SetFixedImage(FixedImageTypePointer I)
    { 
      m_FixedImage = I; 
    }
  void SetImageInterpolator(ImageInterpolatorTypePointer I)
    {
      m_ImageInterpolator = I;
    }
  void SetMetricRadius(RadiusType R) 
    {
      m_MetricRadius = R; 
    }    
   void SetNumberOfIntegrationPoints(unsigned int N)
    { 
      m_NumberOfIntegrationPoints = N;
    }
   unsigned int GetNumberOfIntegrationPoints()
    { 
      return m_NumberOfIntegrationPoints;
    }
  void SetGamma(RealType r) 
    {
      m_Gamma = r;
    }
  void SetSolution(Solution::ConstPointer S) 
    {  
      m_Solution = S; 
    }
  Solution::ConstPointer GetSolution() 
    {  
      return m_Solution; 
    }
  void SetCurrentEnergy(RealType E)
    {
      m_Energy = E;
    }
  RealType GetCurrentEnergy() 
    {
      return m_Energy;
    }
  void SetMaximizeMetric(bool b)
    {
      m_MaximizeMetric = b;
    }
  bool GetMaximizeMetric() 
    {
      return m_MaximizeMetric;
    }
  void SetDeformationField(DeformationFieldTypePointer F) 
    { 
      m_DeformationField = F; 
    }
  DeformationFieldTypePointer GetDeformationField() 
    { 
      return m_DeformationField; 
    }

protected:

private:
  MovingImageTypePointer                                  m_MovingImage;
  FixedImageTypePointer                                   m_FixedImage;
  RadiusType                                              m_MetricRadius;
  unsigned int                                            m_NumberOfIntegrationPoints;

  RealType                                                m_Gamma;
  RealType                                                m_Energy;

  bool                                                    m_MaximizeMetric;
  MetricBaseTypePointer                                   m_Metric;
  ImageInterpolatorTypePointer                            m_ImageInterpolator;
  typename Solution::ConstPointer                         m_Solution;
  DeformationFieldTransformTypePointer                    m_Transform;
  DeformationFieldTypePointer                             m_DeformationField;

  /** Dummy static int that enables automatic registration
      with FEMObjectFactory. */
  static const int DummyCLID;
};
}} // end namespace fem/itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFEMImageMetricLoad.hxx"
#endif

#endif

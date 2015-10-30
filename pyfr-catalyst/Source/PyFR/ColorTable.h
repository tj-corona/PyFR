#ifndef COLORTABLE_H
#define COLORTABLE_H

#define BOOST_SP_DISABLE_THREADS

#include <cassert>
#include <vector>

#include <vtkm/Types.h>
#include <vtkm/VecVariable.h>
#include <vtkm/VectorAnalysis.h>

typedef vtkm::Vec<vtkm::UInt8,4> Color;

// Redefine Lerp to avoid subtraction because Color has unsigned elements
VTKM_EXEC_CONT_EXPORT
Color Lerp(const Color& color0,
           const Color& color1,
           const float& weight)
{
  Color result;
  for (vtkm::IdComponent i=0;i<4;i++)
    {
    result[i] = (1. - weight)*color0[i] + weight*color1[i];
    }
  return result;
}

class ColorTable
{
  enum { MaxSize = 5 };

  public:
  ColorTable()
  {
    this->NumberOfColors = MaxSize;
    this->Palette[0] = Color(255,0,0,255);
    this->Palette[1] = Color(255,255,0,255);
    this->Palette[2] = Color(0,255,0,255);
    this->Palette[3] = Color(0,255,255,255);
    this->Palette[4] = Color(0,0,255,255);
    this->Min = this->Max = 1.;
    this->AssignPivots();
  }

  void SetRange(FPType min,FPType max)
  {
    this->Min = min;
    this->Max = max;
    this->AssignPivots();
  }

  void SetNumberOfColors(vtkm::IdComponent nColors)
  {
    this->NumberOfColors = nColors;
    this->AssignPivots();
  }

  void SetPaletteColor(vtkm::IdComponent i,const Color& color)
  {
    assert(i<MaxSize);
    this->Palette[i] = color;
    this->AssignPivots();
  }

VTKM_EXEC_CONT_EXPORT
  Color operator()(const FPType& value) const
  {
    vtkm::IdComponent index = 1;
    while (index < this->NumberOfColors && this->Pivots[index] < value)
      {
      index++;
      }

    float weight = (value - this->Pivots[index-1])/this->Interval;

    return Lerp(this->Palette[index-1],this->Palette[index],weight);
  }

  VTKM_EXEC_CONT_EXPORT
  FPType operator()(const Color& color) const
  {
    // This doesn't have to be efficient. It is for testing purposes only.

    float weight;
    for (vtkm::IdComponent index = 0;index<this->NumberOfColors-1;index++)
      {
      if (this->ColorIsInInterval(color,index,weight))
        {
        return (1.-weight)*this->Pivots[index] + weight*this->Pivots[index+1];
        }
      }
    return this->Pivots[this->NumberOfColors-1];
  }

  protected:
VTKM_EXEC_CONT_EXPORT
  void AssignPivots()
  {
    this->Interval = (this->Max-this->Min)/(this->NumberOfColors-1.);
    for (vtkm::IdComponent i=0;i<this->NumberOfColors;i++)
      {
      this->Pivots[i] = this->Min + static_cast<float>(i)*this->Interval;
      }
  }

VTKM_EXEC_CONT_EXPORT
  bool ColorIsInInterval(const Color& color,
                         vtkm::IdComponent interval,
                         float& weight) const
  {
    if (color == this->Palette[interval])
      {
      weight = 0.;
      return true;
      }

    typedef vtkm::Vec<float,4> SignedColor;
    SignedColor c0(this->Palette[interval]);
    SignedColor c1(this->Palette[interval+1]);
    SignedColor c(color);

    SignedColor c0_to_c(vtkm::Normal(c - c0));
    SignedColor c0_to_c1(vtkm::Normal(c1 - c0));

    SignedColor::ComponentType dot = c0_to_c.Dot(c0_to_c1);

    // Color::ComponentType max(-1);
    Color::ComponentType max(~0);

    if (vtkm::Abs(dot-1.) > 1./max)
      {
      weight = -1.;
      return false;
      }
    else
      {
      weight = (vtkm::Magnitude(c-c0)/vtkm::Magnitude(c0-c1));
      return true;
      }
  }

  FPType Min;
  FPType Max;
  vtkm::IdComponent NumberOfColors;
  vtkm::Vec<Color,MaxSize> Palette;
  vtkm::Vec<float,MaxSize> Pivots;
  FPType Interval;
};

#endif

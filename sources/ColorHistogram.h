#ifndef COLORHISTOGRAM_H
#define COLORHISTOGRAM_H
#include <vcg/math/histogram.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/bind.h>
#endif

struct ColorJS {
  unsigned char r,g,b,a;
};

namespace vcg
{
/**
 * ColorHistogram.
 *
 * This class implements a single-value histogram.
 */
template <class ScalarType>
    class ColorHistogram : public Histogram<ScalarType>
{

// public data members
private:

  std::vector <Color4f> CV;   //! Counters for color averages.

// public methods
public:
  /**
   * Add a new value to the histogram.
   *
   * The statistics related to the histogram data (average, RMS, etc.) are
   * also updated.
   */
  void Add(ScalarType v,Color4b c,float increment=1.0);

  ColorJS BinColorAvg(ScalarType v) {
    Color4b c = BinColorAvgInd(this->BinIndex(v));
    return ColorJS{c[0], c[1], c[2], 255};
  }

  Color4b BinColorAvgInd(int index) {
    return Color4b( (unsigned char)((CV[index][0] / float(this->H[index]))),
                    (unsigned char)((CV[index][1] / float(this->H[index]))),
                    (unsigned char)((CV[index][2] / float(this->H[index]))),255);
                 }

    //ScalarType RangeCount(ScalarType rangeMin, ScalarType rangeMax);
  //ScalarType BinWidth(ScalarType v);

  //! Reset histogram data.
  void Clear() {
    Histogram<ScalarType>::Clear();
    CV.clear();
  }
  /*
  Note that the histogram holds <n> valid bins plus two semi-infinite bins.

  R[0]   = -inf
  R[1]   = minv
  R[n+1] = maxv
  R[n+2] = +inf


  Eg. SetRange(0, 10, 5) asks for 5 intervals covering the 0..10 range

      H[0]  H[1]   H[2]   H[3]   H[4]   H[5]    H[6]
  -inf    0      2      4      6      8      10    +inf
  R[0]   R[1]   R[2]   R[3]   R[4]   R[5]   R[6]   R[7]

  */
  void SetRange(ScalarType _minv, ScalarType _maxv, int _n, ScalarType gamma=1.0)
  {
    Histogram<ScalarType>::SetRange(_minv,_maxv,_n,gamma);
    CV.resize(_n+2);
    fill(CV.begin(),CV.end(),Color4f(0,0,0,0));
  }
};




/*
    H[0]  H[1]   H[2]   H[3]   H[4]   H[5]    H[6]
-inf    0      2      4      6      8      10    +inf
R[0]   R[1]   R[2]   R[3]   R[4]   R[5]   R[6]   R[7]

asking for 3.14 lower bound will return an iterator pointing to R[3]==4; and will increase H[2]
asking for 4    lower bound will return an iterator pointing to R[3]==4; and will increase H[2]

*/
template <class ScalarType>
void ColorHistogram<ScalarType>::Add(ScalarType v,Color4b c,float increment/*=1.0f*/)
{
  int pos=this->BinIndex(v);
  if(v<this->minElem) this->minElem=v;
  if(v>this->maxElem) this->maxElem=v;
  if(pos>=0 && pos<=this->n)
  {
    CV[pos]+=Color4f(c[0],c[1],c[2],255.0)*increment;
    this->H[pos]+=increment;
    this->cnt+=increment;
    this->sum+=v*increment;
    this->rms+= (v*v)*increment;
  }
}

} // end namespace


typedef vcg::ColorHistogram<float> ColorHistogramf;

#ifdef __EMSCRIPTEN__
using namespace emscripten;
EMSCRIPTEN_BINDINGS(ColorHistogram) {
  value_object<ColorJS>("ColorJS")
    .field("r", &ColorJS::r)
    .field("g", &ColorJS::g)
    .field("b", &ColorJS::b)
    .field("a", &ColorJS::a)
    ;
  class_<vcg::Histogramf>("Histogramf")
    .constructor<>()
    .function<float>("maxV",             &vcg::Histogram<float>::MaxV)
    .function<float>("minV",           &vcg::Histogram<float>::MinV)
    .function<float>("maxCount",         &vcg::Histogram<float>::MaxCountInRange)
    .function("binNum",      &vcg::Histogram<float>::BinNum)
    .function<float>("binCount", select_overload<float(float)>(&vcg::Histogram<float>::BinCount))
    ;
  class_<ColorHistogramf, base<vcg::Histogramf> >("ColorHistogramf")
    .function("binColorAvg", &ColorHistogramf::BinColorAvg)
    ;

}
#endif

#endif // COLORHISTOGRAM_H

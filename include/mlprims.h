#ifndef mlprims_h__
#define mlprims_h__

#include <string>
#include <numeric>
#include <prims.h>
//#include <LU>

using std::string;

namespace ML {

  typedef std::vector<double> dvec;
  typedef std::vector<double> sample;
  typedef std::vector<sample> sample_vec;
  //typedef Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic> dmat;

  inline std::ostream& operator<< (std::ostream& os, const sample& x)
  {
    unsigned n=x.size();
    os << n;
    for(unsigned i=0;i<n;++i)
      os << ' ' << x[i];
    return os;
  }

  inline std::istream& operator>> (std::istream& is, sample& x)
  {
    unsigned n;
    is >> n;
    if (x.size()!=n) x.resize(n);
    for(unsigned i=0;i<n;++i)
      is >> x[i];
    return is;
  }

  inline sample& operator+= (sample& sum, const sample& x)
  {
    sample::iterator sb=sum.begin(),se=sum.end();
    sample::const_iterator b=x.begin(),e=x.end();
    for(;sb!=se && b!=e;++sb,++b)
    {
      *sb += *b;
    }
    return sum;
  }

  inline sample& operator-= (sample& sum, const sample& x)
  {
    sample::iterator sb=sum.begin(),se=sum.end();
    sample::const_iterator b=x.begin(),e=x.end();
    for(;sb!=se && b!=e;++sb,++b)
    {
      *sb -= *b;
    }
    return sum;
  }

  inline sample& operator*= (sample& res, double mult)
  {
    sample::iterator sb=res.begin(),se=res.end();
    for(;sb!=se;++sb) *sb *= mult;
    return res;
  }

  inline double inner_product(const sample& a, const sample& b)
  {
    return std::inner_product(a.begin(),a.end(),b.begin(),0.0);
  }

  inline double eucledian_distance(const sample& a, const sample& b)
  {
    sample x=a;
    x-=b;
    return sqrt(inner_product(x,x));
  }

  inline double hamming_distance(const sample& a, const sample& b)
  {
    double sum=0;
    for(unsigned i=0;i<a.size();++i)
      sum+=fabs(a[i]-b[i]);
    return sum;
  }

  template<class T>
  inline string S(const T& t)
  {
    std::ostringstream os;
    os << t;
    return os.str();
  }

  /*
  template<>
  inline string S(const dmat& m)
  {
    std::ostringstream os;
    unsigned w=m.cols(),h=m.rows();
    os << w << ' ' << h;
    for(unsigned y=0;y<h;++y)
      for(unsigned x=0;x<w;++x)
        os << ' ' << m(y,x);
    return os.str();
  }
  */

  inline sample parse_vector(const string& s)
  {
    std::istringstream is(s);
    sample res;
    is >> res;
    return res;
  }

  /*
  inline dmat parse_matrix(const string& s)
  {
    dmat m;
    int w,h;
    std::istringstream is(s);
    is >> w >> h;
    m.resize(h,w);
    for(int y=0;y<h;++y)
      for(int x=0;x<w;++x)
        is >> m(y,x);
    return m;
  }
  */

  struct TrainingSample
  {
    TrainingSample() : y(0) {}
    TrainingSample(const sample& sx, double sy) : x(sx), y(sy) {}
    sample x;
    double y;
  };

  class ClassifierFeature
  {
  public:
    virtual ~ClassifierFeature() {}
    virtual double calculate(const sample& x) const = 0;
  };

  class Classifier
  {
  public:
    virtual ~Classifier() {}
    virtual void add_sample(const sample& x, bool positive) = 0;
    virtual void finalize() = 0;
    virtual bool classify(const sample& x) const = 0;
  };

  class MultiClassifier
  {
  public:
    virtual ~MultiClassifier() {}
    virtual void add_sample(const sample& x, int class_index) = 0;
    virtual void finalize() = 0;
    virtual int  classify(const sample& x) const = 0;
  };

  typedef double (*kernel_func)(const sample& x, const sample& z);

  inline double rbf_kernel(const sample& x, const sample& z)
  {
    double sum=0;
    sample::const_iterator b=x.begin(),e=x.end(),it=z.begin();
    for(;b!=e;++b,++it)
		sum+=sqr(*b-*it);
    return exp(-sum/8);
  }

} // namespace ML


#endif // mlprims_h__

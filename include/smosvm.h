#ifndef H_SMOSVM_CLASSIFIER
#define H_SMOSVM_CLASSIFIER

#include <mlprims.h>
#include <iostream>
#include <fstream>
#include <xml.h>

namespace ML {

class SMOSVMClassifier : public Classifier
{
  typedef std::vector<int> ivec;
  typedef std::vector<dvec> rows_vec;
  rows_vec    m_PreCalcKernels;
  sample_vec  m_TrainX;
  ivec        m_TrainY;
  kernel_func m_Kernel;
  double      m_Bias;
  dvec        m_Errors;
  double      m_C;

  sample_vec  m_SupportVectors;
  ivec        m_SupportLabels;
  dvec        m_SupportAlpha;

  void calc_row(int i)
  {
    int n=m_TrainY.size();
    double yi=m_TrainY[i];
    dvec& row=m_PreCalcKernels[i];
    row.resize(n);
    for(int j=0;j<n;++j)
      row[j]=m_TrainY[j]*yi*m_Kernel(m_TrainX[i],m_TrainX[j]);
  }

  double K(int i, int j)
  {
    if (i==j) return 1.0;
    if (!m_PreCalcKernels[i].empty()) 
      return (m_PreCalcKernels[i])[j];
    if (!m_PreCalcKernels[j].empty())
      return (m_PreCalcKernels[j])[i];
    calc_row(i);
    return (m_PreCalcKernels[i])[j];
  }

  double calc_error(int i) const
  {
    double yi=m_TrainY[i];
    return m_Errors[i]*yi+m_Bias-yi;
  }

  void modify_alpha(dvec& a, int i, double nai, int n)
  {
    double d=(nai-a[i]);
    dvec& row=m_PreCalcKernels[i];
    if (row.empty()) calc_row(i);
    for(int j=0;j<n;++j)
      m_Errors[j]+=row[j]*d;
    a[i]=nai;
  }
public:
  SMOSVMClassifier(kernel_func k=rbf_kernel)
    : m_Kernel(k),
      m_C(100)
  {}

  void set_c(double C)
  {
    m_C=C;
  }
  
  void load(xml_element* root)
  {
    m_Bias=atof(root->get_attribute("bias").c_str());
    m_PreCalcKernels.clear();
    m_TrainX.clear();
    m_TrainY.clear();
    m_SupportAlpha.clear();
    m_SupportLabels.clear();
    m_SupportVectors.clear();
    xml_element::const_iterator b=root->begin(),e=root->end();
    for(;b!=e;++b)
    {
      xml_element* sv=*b;
      m_SupportAlpha.push_back(atof(sv->get_attribute("alpha").c_str()));
      m_SupportLabels.push_back(atoi(sv->get_attribute("label").c_str()));
      m_SupportVectors.push_back(parse_vector(sv->get_attribute("vector")));
    }

	/*for (sample_vec::iterator itr = m_SupportVectors.begin();
		itr != m_SupportVectors.end();
		++itr)
	{
		cout << " vec = " << *itr << endl;
	}*/
  }
  /*
    loads the classifier file. if no file was found (or some other error) return false, else true.
  */
  virtual bool load(const char* filename)
  {
    std::ifstream fin(filename);
    if (!fin.fail())
    {
      load(fin);
      return true;
    }
    return false;
    
  }

  virtual void load(std::istream& is)
  {
    xml_element* root=xml_parser().parse(is);
    load(root);
  }

  xml_element* save() const
  {
    xml_element* root=new xml_element("smosvm");
    root->set_attribute("bias",S(m_Bias));
    int n=m_SupportVectors.size();
    for(int i=0;i<n;++i)
    {
      xml_element* sv=root->add_child("support");
      sv->set_attribute("vector",S(m_SupportVectors[i]));
      sv->set_attribute("label",S(m_SupportLabels[i]));
      sv->set_attribute("alpha",S(m_SupportAlpha[i]));
    }
    return root;
  }

  xml_element* save(int i) const
  {
    return save();
  }

  virtual void save(std::ostream& os) const
  {
    xml_element* root=save();
    root->print(os,0,false);
    delete root;
  }

  virtual void add_sample(const sample& x, bool positive)
  {
    m_TrainX.push_back(x);
    m_TrainY.push_back(positive?1:-1);
  }

  virtual void finalize()
  {
    int n=m_TrainX.size();
    if (n==0) return;
    m_SupportAlpha.clear();
    m_SupportLabels.clear();
    m_SupportVectors.clear();
    m_PreCalcKernels.resize(n);
//     for(int i=0;i<n;++i)
//     {
//       double yi=m_TrainY[i];
//       dvec& row=m_PreCalcKernels[i];
//       row.resize(n);
//       for(int j=0;j<n;++j)
//         row[j]=m_TrainY[j]*yi*m_Kernel(m_TrainX[i],m_TrainX[j]);
//     }
    int no_change=0;
    m_Bias=0;
    double tolerance=0.1;
    double C=m_C;
    dvec a(n,0.0);
    m_Errors.resize(n,0.0);
    for(int iter=0;no_change<32;++iter)
    {
      //std::cout << "Iteration: " << iter << "\r";
      int mod_alphas=0;
      for(int i=0;i<n;++i)
      {
        double sum=0;
        double yi=m_TrainY[i];
        double Ei=calc_error(i);
        if ((yi*Ei<(-tolerance) && a[i]<C) ||
            (yi*Ei>tolerance && a[i]>0))
        {
          int j=i;
          while (j==i) j=rand()%n;
          double yj=m_TrainY[j];
          double Ej=calc_error(j);
          double ai=a[i],aj=a[j];
          double L,H;
          if (yi==yj)
          {
            L=Max(0.0,ai+aj-C);
            H=Min(C,ai+aj);
          }
          else
          {
            L=Max(0.0,aj-ai);
            H=Min(C,C+aj-ai);
          }
          if (L!=H)
          {
            double eta=2*yi*yj*K(i,j)-2;
            if (eta<0)
            {
              double naj=aj-yj*(Ei-Ej)/eta;
              naj=Min(naj,H);
              naj=Max(naj,L);
              if (fabs(naj-aj)>0.00001)
              {
                double nai=ai+yi*yj*(aj-naj);
                double b1=m_Bias-Ei-yi*(nai-ai)*K(i,i)-yi*(naj-aj)*K(i,j);
                double b2=m_Bias-Ej-yj*(nai-ai)*K(i,j)-yj*(naj-aj)*K(j,j);
                if (nai>0.0 && nai<C) m_Bias=b1;
                else
                if (naj>0.0 && naj<C) m_Bias=b2;
                else
                m_Bias=0.5*(b1+b2);
                ++mod_alphas;
                modify_alpha(a,i,nai,n);
                modify_alpha(a,j,naj,n);
              }
            }
          }
        }
      }
      if (mod_alphas>0) no_change=0;
      else ++no_change;
    }
    for(int i=0;i<n;++i)
    {
      if (a[i]>0)
      {
        m_SupportVectors.push_back(m_TrainX[i]);
        m_SupportLabels.push_back(m_TrainY[i]);
        m_SupportAlpha.push_back(a[i]);
      }
    }
    m_TrainX.clear();
    m_TrainY.clear();
    m_PreCalcKernels.clear();
  }

  bool classify(const sample& x, double& conf) const
  {
    conf=m_Bias;
    int n=m_SupportVectors.size();
    for(int i=0;i<n;++i)
    {
		double temp = (m_SupportAlpha[i]*m_SupportLabels[i]*m_Kernel(m_SupportVectors[i],x));
//		cout << " x = " << x << endl;
      conf+=temp;
    }
    return conf>=0;
  }

  virtual bool classify(const sample& x) const
  {
    double conf=0.0;
    return classify(x,conf);
  }
};

} // namespace ML

#endif // H_SMOSVM_CLASSIFIER

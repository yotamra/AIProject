#ifndef feature_manager_h__
#define feature_manager_h__

#include <memory>
#include <string>
#include <list>
#include <image.h>
#include <xml.h>
#include <stringtokenizer.h>
#include <mlprims.h>

using namespace CPP;
using std::string;

using ML::sample;
using ML::sample_vec;

template<class T>
inline string S(const T& t)
{
  std::ostringstream os;
  os << t;
  return os.str();
}

typedef std::vector<bool> bool_vec;
typedef std::vector<string> str_vec;

class pos_image
{
  Image  image;
  int    frame;
  double x,y;
  Image  scaled;
  double sx,sy;
public:
  pos_image() : x(0), y(0), frame(0) {}
  pos_image(double dx, double dy, int fr, Image img)
    : x(dx), y(dy), frame(fr), image(img), scaled(img),
      sx(dx), sy(dy)
  {}

  void  scale_image(double reduction_ratio)
  {
    int w=image->get_width(),h=image->get_height();
    scaled=scale_to(image,int(w/reduction_ratio),int(h/reduction_ratio));
    scaled=threshold(scaled,128);
    sx=x/reduction_ratio;
    sy=y/reduction_ratio;
  }

  Image get_image()
  {
    return scaled;
  }

  int    get_frame() const { return frame; }
  double get_x() const { return sx; }
  double get_y() const { return sy; }

  bool operator< (const pos_image& rhs) const { return get_frame()<rhs.get_frame(); }
};

typedef std::list<pos_image> image_list;

class Feature
{
public:
  virtual ~Feature() {}
  virtual string calculate(pos_image image) = 0;
};

class SequenceFeature
{
public:
  virtual ~SequenceFeature() {}
  virtual string calculate(image_list& imgs, xml_element* tree) = 0;
};

#define FEATURE(x) class Feature_##x : public Feature { public:\
Feature_##x() { FeatureManager::instance()->register_feature(#x,this); }\
virtual string calculate(pos_image pimg);\
} g_Feature_##x; string Feature_##x::calculate(pos_image pimg)

#define SEQUENCE_FEATURE(x) class Feature_##x : public SequenceFeature { public:\
Feature_##x() { FeatureManager::instance()->register_feature(#x,this); }\
virtual string calculate(image_list& images, xml_element* tree);\
} g_Feature_##x; string Feature_##x::calculate(image_list& images, xml_element* tree)

class FeatureManager
{
public:
  static FeatureManager* instance()
  {
    static std::auto_ptr<FeatureManager> ptr(new FeatureManager);
    return ptr.get();
  }

  void register_feature(const string& name, Feature* f)
  {
    m_Features[name]=f;
  }

  void register_feature(const string& name, SequenceFeature* f)
  {
    m_SequenceFeatures[name]=f;
  }

  string calculate(Image image, const string& feature_name, int scale)
  {
    feature_map::iterator it=m_Features.find(feature_name);
    if (it==m_Features.end()) return "N/A";
    pos_image p(0,0,0,image);
    if (scale>1) p.scale_image(scale);
    return it->second->calculate(p);
  }

  void calculate_all(pos_image image, sample& x)
  {
    feature_map::iterator b=m_Features.begin(),e=m_Features.end();
    for(;b!=e;++b)
    {
      string name=b->first;
      Feature* f=b->second;
      string value=f->calculate(image);
      string_tokenizer st(value,",");
      if (st.size()>1)
      {
        string_tokenizer::const_iterator b=st.begin(),e=st.end();
        for(int i=1;b!=e;++b,++i)
        {
          value=*b;
          x.push_back(atof(value.c_str()));
        }
      }
      else
      {
        x.push_back(atof(value.c_str()));
      }
    }
  }

  void calculate_all(pos_image image, xml_element* el)
  {
    if (m_Selection.empty()) m_Selection.assign(1024,true);
    int n=0,index=0;
    feature_map::iterator b=m_Features.begin(),e=m_Features.end();
    for(;b!=e;++b)
    {
      string name=b->first;
      Feature* f=b->second;
      string value=f->calculate(image);
      string_tokenizer st(value,",");
      if (st.size()>1)
      {
        string_tokenizer::const_iterator b=st.begin(),e=st.end();
        for(int i=1;b!=e;++b,++i,++n)
        {
          if (index>=int(m_FeatureNames.size()))
            m_FeatureNames.push_back(name+S(i));
          if (m_Selection[index++])
          {
            value=*b;
            xml_element* fel=el->add_child("feature");
            fel->set_attribute("name",name+"_"+S(i));
            fel->set_attribute("value",value);
          }
        }
      }
      else
      {
        ++n;
        if (index>=int(m_FeatureNames.size()))
          m_FeatureNames.push_back(name);
        if (m_Selection[index++])
        {
          xml_element* fel=el->add_child("feature");
          fel->set_attribute("name",name);
          fel->set_attribute("value",value);
        }
      }
    }
    if (m_StaticFeaturesCount==0)
      m_StaticFeaturesCount=n;
  }

  void calculate_sequence_features(image_list& images, sample_vec& seq)
  {
    xml_element root("sequence");
    for(image_list::iterator it=images.begin();it!=images.end();++it)
    {
      pos_image& pi=*it;
      calculate_all(pi,root.add_child("sil"));
    }
    calculate_sequence_features(images,&root);
    sample basex,x;
    for(xml_element::attr_iterator ab=root.attr_begin(),ae=root.attr_end();ab!=ae;++ab)
    {
      if (ab->first!="height")
      {
        string_tokenizer st(ab->second," ");
        string_tokenizer::const_iterator sb=st.begin(),se=st.end();
        for(;sb!=se;++sb)
        {
          string svalue=*sb;
          double value=atof(svalue.c_str());
          basex.push_back(value);
        }
      }
    }
    for(xml_element::iterator xb=root.begin(),xe=root.end();xb!=xe;++xb)
    {
      xml_element* frame=*xb;
      x.clear();
      for(xml_element::iterator b=frame->begin(),e=frame->end();b!=e;++b)
      {
        xml_element* feature=*b;
        x.push_back(atof(feature->get_attribute("value").c_str()));
      }
      x.insert(x.end(),basex.begin(),basex.end());
      seq.push_back(x);
    }
  }

  void calculate_sequence_features(image_list& images, sample& sx)
  {
    seq_feature_map::iterator b=m_SequenceFeatures.begin(),e=m_SequenceFeatures.end();
    for(;b!=e;++b)
    {
      SequenceFeature* f=b->second;
      string_tokenizer st(f->calculate(images,0));
      for(string_tokenizer::const_iterator it=st.begin();it!=st.end();++it)
      {
        sx.push_back(atof(it->c_str()));
      }
    }
  }

  void calculate_sequence_features(image_list& images, xml_element* root)
  {
    seq_feature_map::iterator b=m_SequenceFeatures.begin(),e=m_SequenceFeatures.end();
    int index=m_StaticFeaturesCount;
    for(;b!=e;++b)
    {
      string name=b->first;
      SequenceFeature* f=b->second;
      string value=f->calculate(images,root);
      string_tokenizer st(value);
      if (st.size()>1)
      {
        int i=1;
        for(string_tokenizer::const_iterator it=st.begin();it!=st.end();++it,++i)
        {
          if (index>=int(m_FeatureNames.size()))
            m_FeatureNames.push_back(name+S(i));
          if (m_Selection[index++])
            root->set_attribute(name+S(i),*it);
        }
      }
      else
      {
        if (index>=int(m_FeatureNames.size()))
          m_FeatureNames.push_back(name);
        if (m_Selection[index++])
          root->set_attribute(name,value);
      }
    }
  }

  void calculate_group_sizes()
  {
    Image img(new CPPImage(32,32,1));
    fill(img,255);
    xml_element root("seq");
    pos_image pimage(0,0,0,img);
    {
      calculate_all(pimage,root.add_child("sil"));
      feature_map::iterator b=m_Features.begin(),e=m_Features.end();
      for(;b!=e;++b)
      {
        string name=b->first;
        Feature* f=b->second;
        string value=f->calculate(pimage);
        string_tokenizer st(value,",");
        m_GroupSizes.push_back(st.size());
      }
    }
    image_list l;
    l.push_back(pimage);
    l.push_back(pimage);
    {
      seq_feature_map::iterator b=m_SequenceFeatures.begin(),e=m_SequenceFeatures.end();
      for(;b!=e;++b)
      {
        string name=b->first;
        SequenceFeature* sf=b->second;
        string value=sf->calculate(l,&root);
        string_tokenizer st(value,",");
        m_GroupSizes.push_back(st.size());
      }
    }
  }

  void set_feature_selection(const bool_vec& v, bool print=false)
  {
    m_Selection=v;
    if (print)
    {
      int i=0;
      for(bool_vec::const_iterator it=v.begin();it!=v.end();++it,++i)
      {
        std::cout << i << "\t";
        if (*it) std::cout << "*"; 
        std::cout << "\t" << get_feature_name(i) << std::endl;
      }
    }
  }

  const bool_vec& reset_feature_selection()
  {
    if (m_GroupSizes.empty()) calculate_group_sizes();
    int n=std::accumulate(group_begin(),group_end(),0);
    m_Selection.assign(n,true);
    return m_Selection;
  }

  void initialize_features()
  {
    Image image(new CPPImage(32,32,1));
    fill(image,255);
    pos_image pi(0,0,0,image);
    image_list il;
    il.push_back(pi);
    il.push_back(pi);
    xml_element root("root");
    xml_element* sil=root.add_child("sil");
    calculate_all(pi,sil);
    sil=root.add_child("sil");
    calculate_all(pi,sil);
    calculate_sequence_features(il,&root);
  }

  typedef std::vector<int> int_vec;
  int_vec::const_iterator group_begin() const { return m_GroupSizes.begin(); }
  int_vec::const_iterator group_end()   const { return m_GroupSizes.end(); }

  unsigned get_features_number()
  {
    if (m_FeatureNames.empty()) initialize_features();
    return m_FeatureNames.size(); 
  }
  string   get_feature_name(int feature) const { return m_FeatureNames[feature]; }
private:
  friend class std::auto_ptr<FeatureManager>;
  FeatureManager() 
    : m_StaticFeaturesCount(0)
  {}
  ~FeatureManager() {}
  FeatureManager(const FeatureManager&) {}
  FeatureManager& operator= (const FeatureManager&) { return *this; }

  typedef std::map<string,Feature*> feature_map;
  typedef std::map<string,SequenceFeature*> seq_feature_map;

  int             m_StaticFeaturesCount;
  bool_vec        m_Selection;
  int_vec         m_GroupSizes;
  str_vec         m_FeatureNames;
  feature_map     m_Features;
  seq_feature_map m_SequenceFeatures;
};


#endif // feature_manager_h__

#ifndef PERSON_CLASSIFIER
#define PERSON_CLASSIFIER

#include "cc.h"
#include "image.h"

#include <smosvm.h>
#include <feature_manager.h>

using namespace ML;

class TargetEvaluator
{
public:
  virtual ~TargetEvaluator() {}
  virtual double evaluate(CPP::Image image) = 0;
};

#include "clseval.h"

inline void calculate_features(Image image, sample& x)
{
  x.clear();
  image=convert_to_gray(image);
#ifdef DIFF_MAP
  const char* fnames[] = { "Rays"};
#else
  //const char* fnames[] = { /*"Rays", "SPPA",*/ "Invar", "Legs" };
  const char* fnames[] = { "Rays", "Legs" };
#endif
  int n=sizeof(fnames)/sizeof(const char*);
  for(int i=0;i<n;++i)
  {
    string_tokenizer st(FeatureManager::instance()->calculate(image,fnames[i],0)," \t,;");
    for(string_tokenizer::const_iterator sit=st.begin();sit!=st.end();++sit)
    {
      x.push_back(atof(sit->c_str()));
    }
  }
}

class CPersonClassifier
{
public:
	// returns the persons from the candidates
	static cc_list classify(cc_list& candidates,TargetEvaluator* te);
};

#endif
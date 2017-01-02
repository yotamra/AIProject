#ifndef clseval_h__
#define clseval_h__

void calculate_features(Image image, sample& x);

class ClsEvaluator : public TargetEvaluator
{
  ML::SMOSVMClassifier& m_Classifier;
public:
  ClsEvaluator(ML::SMOSVMClassifier& cls) : m_Classifier(cls) {}
  double evaluate(CPP::Image image)
  {
    sample x;
    double conf;
    calculate_features(image,x);
    m_Classifier.classify(x,conf);
    return conf;
  }
};

#endif // clseval_h__

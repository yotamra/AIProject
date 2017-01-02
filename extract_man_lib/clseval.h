#ifndef clseval_h__
#define clseval_h__

void calculate_features(Image image, sample& x);

/*
A class for handling the svm algorithm.
*/
class ClsEvaluator : public TargetEvaluator
{
  ML::SMOSVMClassifier& m_Classifier;
public:
  //initialization with the xmk file of the classifier.
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

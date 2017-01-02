#include "../libsensor/sensor.h"
#include <iomanip>
#include <cmdline.h>
#include <files.h>
#include <smosvm.h>
#include <feature_manager.h>
#include <profiler.h>
#include <statistics.h>
#include <threads.h>
#include "moments.h"
#include "clseval.h"
#include "pinpointing.h"
#include "CFollow.h"

using namespace CPP;
using namespace ML;

bool   g_DataCollection=false;
bool   g_Train=false;
string g_Frame;
//#define DIFF_MAP

//Image extract_man(Image img, TargetEvaluator* te, int& off_x, int& off_y);

//void calculate_features(Image image, sample& x)
//{
//  x.clear();
//  image=convert_to_gray(image);
//#ifdef DIFF_MAP
//  const char* fnames[] = { "Rays"};
//#else
//  //const char* fnames[] = { /*"Rays", "SPPA",*/ "Invar", "Legs" };
//  const char* fnames[] = { "Rays", "Legs" };
//#endif
//  int n=sizeof(fnames)/sizeof(const char*);
//  for(int i=0;i<n;++i)
//  {
//    string_tokenizer st(FeatureManager::instance()->calculate(image,fnames[i],0)," \t,;");
//    for(string_tokenizer::const_iterator sit=st.begin();sit!=st.end();++sit)
//    {
//      x.push_back(atof(sit->c_str()));
//    }
//  }
//}

/*
Collects and saves images from the camera as png files.
*/
void collect_data(Sensor* camera)
{
  unsigned stop,start=GetTickCount();
  int i=0;
  bool personFound;
  Image colorFrame(new CPPImage(640,480,3)),depthFrame;
  SMOSVMClassifier cls;
  cls.load("classifier.xml");
  ClsEvaluator te(cls);
  for(stop=start;stop<(start+30000);stop=GetTickCount())
  {
    int x,y;
    camera->update(colorFrame,depthFrame);
    Image image=camera->extract_man(colorFrame,depthFrame,&te,x,y,personFound);
	if (personFound)
	{
		std::ostringstream os;
		os << "sample_" << std::setfill('0') << std::setw(4) << ++i << ".png";
		image->save(os.str());
		cout << "taking sample number " << i << endl;
	}
  }
}

/*
Finds the image files for the training and calcs their features and saves them to the vectors for the SVM.
*/
void process_files(const string& spec, bool label, sample_vec& samples, bool_vec& labels)
{
  str_vec names;
  string dir=spec;
  int p=dir.find_last_of('\\');
  if (p>=0) dir=dir.substr(0,p+1);
  find_files(spec,names);
  int i=0;
  for(str_vec::iterator it=names.begin();it!=names.end();++it)
  {
    sample x;
    string path=dir+*it;
    std::cout << "Processing " << path << "                            \r";
    Image image=load(path);
    {
      string d="train";
      if ((++i%10)==2) d="test";
//       std::ostringstream os;
//       os << d << "\\" << (label?"man":"noise") << "\\sample_"
//         << std::setfill('0') << std::setw(4) << i << ".png";
//       image->save(os.str());
    }
    calculate_features(image,x);
    samples.push_back(x);
    labels.push_back(label);
  }
}

/*
Not sure, but probably checks if the good and bad examples given to classifier are recognized as good and bad like they should.
*/
void cross_validation(const sample_vec& samples, const bool_vec& labels)
{
  Profiler prof;
  dvec scores;
  int n=samples.size();
  for(int i=0;i<10;++i)
  {
#ifdef DIFF_MAP
    BinaryDiffusionMap cls;
#else
    SMOSVMClassifier cls;
    cls.set_c(500);
#endif
    for(int j=0;j<n;++j)
      if ((j%10)!=i) cls.add_sample(samples[j],labels[j]);
    cls.finalize();
    int good=0,total=0,false_pos=0;
    for(int j=0;j<n;++j)
    {
      if ((j%10)==i)
      {
        ++total;
        bool expected=labels[j];
        bool res=cls.classify(samples[j]);
        if (res==expected) ++good;
        else
        if (res && !expected) ++false_pos;
      }
    }
    double score=double(good)/total;
    double fp_rate=double(false_pos)/total;
    std::cout << i << "\tScore=" << score << "  False Positives=" << fp_rate << std::endl;
    scores.push_back(score);
  }
  std::cout << "Average score=" << calculate_average(scores.begin(),scores.end()) << std::endl;
  prof.print(std::cout,"Cross Validation");
}

/*
Creates the classifier file from the good and bad image examples.
*/
void perform_training()
{
  sample_vec samples;
  bool_vec labels;
  process_files("C:\\temp\\good\\*.jpg",true,samples,labels);
  process_files("C:\\temp\\bad\\*.jpg",false,samples,labels);
  std::cout << std::endl;
  cross_validation(samples,labels);
  {
    Profiler prof;
    SMOSVMClassifier svm;
    svm.set_c(500);
    int n=samples.size();
    for(int i=0;i<n;++i)
      svm.add_sample(samples[i],labels[i]);
    svm.finalize();
    std::ofstream fout("classifier.xml");
    svm.save(fout);
    prof.print(std::cout,"Full Training");
  }
}

//forward declaration
void chase();

int main(int argc, char* argv[])
{
	CFollow cFollow;
	bool training = false;
	if(training)
	{
		perform_training();
	}
  else
  {
	  cFollow.Init();
	  cFollow.RunningAndFollow();
  }
  return 0;
}

COMMAND_LINE_OPTION(c,false,"Collect image data")
{
  g_DataCollection=true;
}

COMMAND_LINE_OPTION(t,false,"Perform training")
{
  g_Train=true;
}

COMMAND_LINE_OPTION(f,true,"Process frame image")
{
  g_Frame=param;
}


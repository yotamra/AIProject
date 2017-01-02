#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "image.h"
#include "imageutils.h"
#include "stringtokenizer.h"

using namespace CPP;
using std::string;

Image load_ppm(const string& filename)
{
  std::ifstream fin(filename.c_str(),std::ios::in|std::ios::binary);
  if (fin.fail()) throw string("File not found: "+filename);
  char line[256];
  fin.getline(line,256,'\n');
  fin.getline(line,256,'\n');
  string_tokenizer st(line," ");
  string_tokenizer::const_iterator it=st.begin();
  int w=atoi((*it++).c_str());
  int h=atoi((*it++).c_str());
  Image img(new CPPImage(w,h,1));
  fin.getline(line,256,'\n');
  for(int y=0;y<h;++y)
  {
    byte* row=img->get_row(y);
    for(int x=0;x<w;++x)
    {
      byte color[3];
      fin.read((char*)color,3);
      row[x]=color[0];
    }
  }
  return img;
}

void save_ppm(const string& filename, Image img)
{
  std::ofstream fout(filename.c_str(),std::ios::out|std::ios::binary);
  std::ostringstream os;
  os << "P6\n" << img->get_width() << " " << img->get_height() << "\n255\n";
  string header=os.str();
  fout.write(header.c_str(),header.size());
  const unsigned char white[] = { 255,255,255 };
  const unsigned char black[] = { 0,0,0 };
  unsigned w=img->get_width(),h=img->get_height(),x,y;
  for(y=0;y<h;++y)
  {
    const byte* row=img->get_row(y);
    for(x=0;x<w;++x)
    {
      const char* color=(const char*)(row[x]?white:black);
      fout.write(color,3);
    }
  }
}


#include <windows.h>
#include "files.h"

/*
Searches for files with the given path in spec (may include wildcards like *.h) 
and inserts them into the strings vector v. If only_dirs is true so
files that are not dirs are not included.
*/
void find_files(const string& spec, str_vec& v, bool only_dirs)
{
  v.clear();
  WIN32_FIND_DATA fd;
  HANDLE h=FindFirstFile(spec.c_str(),&fd);
  if (h!=INVALID_HANDLE_VALUE)
  {
    do
    {
      string filename=fd.cFileName;
      if (filename=="." || filename=="..") continue;
      bool is_dir=(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
      if (is_dir==only_dirs)
        v.push_back(filename);
    } while (FindNextFile(h,&fd));
  }
}


/*
Creates a directory in the given path name
*/
void create_directory(const string& name)
{
  CreateDirectory(name.c_str(),0);
}

string get_current_directory()
{
  char buffer[256];
  GetCurrentDirectory(256,buffer);
  return buffer;
}
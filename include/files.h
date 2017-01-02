#ifndef H_FILES
#define H_FILES

#include <string>
#include <vector>

using std::string;
typedef std::vector<string> str_vec;

string get_current_directory();
void find_files(const string& spec, str_vec& v, bool only_dirs=false);
void create_directory(const string& name);

#endif // H_FILES


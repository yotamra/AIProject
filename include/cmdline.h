#ifndef cmdline_h__
#define cmdline_h__

#include <string>
#include <memory>
#include <map>
#include <vector>

using std::string;

class CommandLineOption
{
  bool m_TakesParam;
public:
  CommandLineOption() : m_TakesParam(false) {}
  virtual ~CommandLineOption() {}
  virtual void process(const string& param) = 0;

  void set_takes_param(bool state) { m_TakesParam=state; }
  bool takes_param() const { return m_TakesParam; }
};

#define COMMAND_LINE_OPTION(name,takes_param,description)\
class Opt_##name : public CommandLineOption { public:\
Opt_##name() { CommandLine::instance()->register_option(#name,takes_param,this); }\
void process(const string& param); } g_Opt_##name;\
void Opt_##name::process(const string& param)

class CommandLine
{
public:
  static CommandLine* instance()
  {
    static std::auto_ptr<CommandLine> ptr(new CommandLine);
    return ptr.get();
  }

  void register_option(const string& name, bool takes_param, CommandLineOption* opt)
  {
    opt->set_takes_param(takes_param);
    m_Options[name]=opt;
  }

  bool process(int argc, char* argv[], const string& usage, unsigned minp, unsigned maxp)
  {
    for(int i=1;i<argc;++i)
    {
      string arg=argv[i];
      if (arg[0]=='-')
      {
        arg=arg.substr(1);
        opt_map::iterator it=m_Options.find(arg);
        if (it==m_Options.end()) throw string("Invalid command line option -"+arg);
        CommandLineOption* opt=it->second;
        string param;
        if (opt->takes_param())
        {
          if (i==(argc-1)) throw string("Missing parameter for option -"+arg);
          param=argv[++i];
        }
        opt->process(param);
      }
      else
        m_Params.push_back(arg);
    }
    if (m_Params.size()<minp || m_Params.size()>maxp)
    {
      std::cout << "Usage: " << argv[0] << " [options] " << usage << std::endl;
      return false;
    }
    return true;
  }

  unsigned get_parameter_count() const { return m_Params.size(); }

private:
  friend class std::auto_ptr<CommandLine>;
  CommandLine() {}
  ~CommandLine() {}
  CommandLine(const CommandLine&) {}
  CommandLine& operator= (const CommandLine&) { return *this; }

  typedef std::map<string,CommandLineOption*> opt_map;
  typedef std::vector<string> str_vec;
  opt_map m_Options;
  str_vec m_Params;
public:
  typedef str_vec::const_iterator const_iterator;
  const_iterator begin() const { return m_Params.begin(); }
  const_iterator end()   const { return m_Params.end(); }

  string get(unsigned index) const
  {
    const_iterator it=begin();
    if (index>0) std::advance(it,index);
    return *it;
  }
};

#define PROCESS_COMMAND_LINE_P(usage,minp,maxp)\
CommandLine* cmd=CommandLine::instance();  if (!cmd->process(argc,argv,usage,minp,maxp)) return 1

#define PROCESS_COMMAND_LINE(usage)\
CommandLine* cmd=CommandLine::instance();  if (!cmd->process(argc,argv,usage,0,99999)) return 1

#endif // cmdline_h__

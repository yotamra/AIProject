#ifndef H_STRING_TOKENIZER
#define H_STRING_TOKENIZER

#include <string>
#include <vector>

using std::string;

template<class T>
class tokenizer_istream
{
  typedef tokenizer_istream<T> self;
  typedef typename T::const_iterator const_iterator;
  T& m_Tokenizer;
  const_iterator m_Iter;
public:
  tokenizer_istream(T& tokenizer) : m_Tokenizer(tokenizer), m_Iter(tokenizer.begin()) {}
  self& operator>> (string& s) { s=*m_Iter++; return *this; }
  self& operator>> (int& i)    { i=atoi((*m_Iter++).c_str()); return *this; }
};

class string_tokenizer
{
  typedef std::vector<string> str_vec;
  str_vec m_Tokens;
public:
  string_tokenizer(const string& s, const string& delim=" ", bool empties=false)
  {
    string::const_iterator it,b=s.begin(),e=s.end();
    string token;
    while (b!=e)
    {
      if (delim.find(*b)!=string::npos) 
      {
        if (empties || !token.empty())
          m_Tokens.push_back(token);
        token.clear();
        ++b; 
        continue; 
      }
      for(it=b;++it!=e;)
        if (delim.find(*it)!=string::npos) break;
      token=string(b,it);
      b=it;
    }
    if (empties || !token.empty()) 
      m_Tokens.push_back(token);
  }

  unsigned size() const { return m_Tokens.size(); }

  typedef str_vec::const_iterator const_iterator;
  const_iterator begin() const { return m_Tokens.begin(); }
  const_iterator end()   const { return m_Tokens.end(); }
  typedef str_vec::const_reverse_iterator const_reverse_iterator;
  const_reverse_iterator rbegin() const { return m_Tokens.rbegin(); }
  const_reverse_iterator rend()   const { return m_Tokens.rend(); }

  tokenizer_istream<string_tokenizer> stream() 
  { 
    return tokenizer_istream<string_tokenizer>(*this);
  }
};


#endif // H_STRING_TOKENIZER


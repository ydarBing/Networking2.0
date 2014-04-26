#include "MainPrecompiled.h"
#include "URIParser.h"

 /* std::string scheme_;
  std::string authority_;
  std::string path_;
  std::string queries_;
  std::string fragments_;
*/

/*****************************************************************************/
//  Private URI parsing helper functions
/*****************************************************************************/

std::string URIParser::ReadUntilFirst(charset& term, std::string& uri, bool consumeLast)
{
  unsigned end = uri.size();
  //Find first terminating character
  for(auto iter = term.begin();iter != term.end();++iter)
  {
    end = min(end, uri.find_first_of(*iter));
  }
  std::string residual = uri.substr(0, end);

  if(consumeLast && uri.size() > 1)
    uri = uri.substr(end + 1, uri.size() - (end + 1));
  else
    uri = uri.substr(end, uri.size() - (end));

  return residual;
}

/*****************************************************************************/
//  Private URI parser functions
/*****************************************************************************/

//Hardcoded, neverchanging, set of reserved chars
/*
    gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"

    sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
                / "*" / "+" / "," / ";" / "="
*/
bool URIParser::IsReservedChar(char c)
{
  if(c == ':' || c == '/' || c == '?' || c == '#' || c == '[' || c == ']' || c == '@' ||
    c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' || c == ')' ||
    c == '*' || c == '+' || c == ',' || c == ';' || c == '=')
    return true;
  return false; 
}

//Decodes all non reserved chars
void URIParser::PercentDecode(std::string& uri)
{
  std::string newuri;
  unsigned old = 0;
  unsigned curr = 0;
  //Seek next percent sign
  curr = uri.find_first_of('%');
  
  //If devoid of %, return
  if(curr == uri.npos)
    return;

  while(curr != uri.npos)
  {
    //Take next two hexidecimal digits
    char f = uri[curr + 1];
    char s = uri[curr + 2];

    //Translate to character
    int fval = (f < 'a' ? f - '0' : (f - 'a') + 10) * 16;
    int sval = f < 'a' ? f - '0' : (f - 'a') + 10;
    char c = fval + sval;

    //Append previous string
    newuri += uri.substr(old, curr - old);
    
    //If non reserved, translate it back to ascii
    if(!IsReservedChar(c))
    {
      //Replace in uri
      newuri += c;
      curr += 3;// Skip over the percent encoded character
    }

    //Find next %
    old = curr;
    curr = uri.find_first_of('%', curr);
  }

  //Append last substr
  newuri += uri.substr(old, curr - old);

  uri = newuri;
}

//Terminated by :
void URIParser::ReadScheme(std::string& uri)
{
  charset term;
  term.push_back(':');
  term.push_back('/');
  term.push_back('?');
  term.push_back('#');
  
  scheme_ = ReadUntilFirst(term, uri, true);
}

//Starts with // terminated by any /#?
void URIParser::ReadAuthority(std::string& uri)
{
  if(uri.size() > 1 && (uri[0] == '/' && uri[1] == '/'))
  {
    //Eat the //
    uri = uri.substr(2, uri.size() - 2);

    charset term;
    term.push_back('/');
    term.push_back('?');
    term.push_back('#');
    
    authority_ = ReadUntilFirst(term, uri);
  }
}

//Assume terminated by query or fragment
//And started by normal char or /
void URIParser::ReadPath(std::string& uri)
{
  if(uri.size() > 0 && (uri[0] != '?' && uri[0] != '#'))
  {
    charset term;
    term.push_back('?');
    term.push_back('#');

    path_ = ReadUntilFirst(term, uri);
  }
}

//Assume terminated by fragments
void URIParser::ReadQueries(std::string& uri)
{
  if(uri.size() > 0 && uri[0] == '?' )
  {
    charset term;
    term.push_back('#');

    query_ = ReadUntilFirst(term, uri);
  }
}

//Assume only fragments are left
void URIParser::ReadFragments(std::string& uri)
{
  if(uri.size() > 0 && uri[0] == '#' )
  {
    query_ = uri;
    uri = "";
  }
}

/*****************************************************************************/
//  Public URI parser functions
/*****************************************************************************/

void URIParser::DecodeURI(std::string uri)
{
  //Bring the uri to lowercase
  //Watch out for potential bugs with this
  for(unsigned i = 0;i < uri.size();++i)
    uri[i] = tolower(uri[i]);

  //Parse the uri
  PercentDecode(uri);
  ReadScheme(uri);
  ReadAuthority(uri);
  ReadPath(uri);
  ReadQueries(uri);
  ReadFragments(uri);
}

Authority URIParser::DecodeAuthority()
{
  std::string sa = authority_;
  Authority auth;
  //Read the userinfo
  //If it exists
  unsigned usrinfEnd = sa.find_first_of('@');
  if(usrinfEnd != sa.npos)
  {
    auth.userdata = sa.substr(0, usrinfEnd);
    sa = sa.substr(usrinfEnd + 1, sa.size() - (usrinfEnd + 1));
  }
  
  //Read the port
  unsigned portBegin = sa.find_first_of(':');//Using find first of is safe b/c userinfo has been lopped off
  if(portBegin != sa.npos)
  {
    auth.port = sa.substr(portBegin + 1, sa.npos - (portBegin + 1));
    sa = sa.substr(0, portBegin);
  }

  //Read the host
  auth.host = sa; //Which should be the leftovers

  return auth;
}

Query URIParser::DecodeQuery()
{
  Query q;
  //Read each key value pair starting with a ?
  std::string query = query_;

  unsigned offset = 0;
  unsigned curr = 0;
  do
  {
    curr = query.find_first_of('?', offset);

    //Get kv pair
    std::string sect = query.substr(offset, curr);

    //Parse kv pair
    if(sect.size() > 0)
    {
      unsigned eq = sect.find_first_of('=');

      std::string key = sect.substr(0, eq);
      std::string value = "";

      if(eq != sect.npos)
        value = sect.substr(eq + 1, sect.npos - (eq + 1));

      q.query.push_back(std::make_pair(key, value));
    }

    offset = curr + 1;

  }while(curr != query.npos);

  return q;
  
}

Path URIParser::DecodePath()
{
  Path p;
  unsigned offset = 0;
  unsigned curr = 0;
  std::string path = path_;
  //Read each segment of the path
  do
  {
    curr = path.find_first_of('/', offset);
    //Add to p list

    if(curr - offset > 0)
      p.segments.push_back(path.substr(offset, curr - offset));

    offset = curr + 1;
  }while(curr != path_.npos);

  return p;
}

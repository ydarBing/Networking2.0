#pragma once
#include <string>
#include <vector>

struct Authority
{
  std::string userdata;
  std::string host;
  std::string port;
};

struct Path
{
  std::vector<std::string> segments;
};

struct Query
{
  std::vector<std::pair<std::string, std::string>> query;
};

class URIParser
{
private:
  typedef std::vector<char> charset;

  std::string scheme_;
  std::string authority_;
  std::string path_;
  std::string query_;
  std::string fragment_;

  bool IsReservedChar(char c);
  void PercentDecode(std::string& uri);

  std::string ReadUntilFirst(charset& term, std::string& uri, bool consumeLast = false);
  void ReadScheme(std::string& uri);
  void ReadAuthority(std::string& uri);
  void ReadPath(std::string& uri);
  void ReadQueries(std::string& uri);
  void ReadFragments(std::string& uri);
public:

  //Splits a URI into a the five generic elements
  void DecodeURI(std::string uri);
  
  //Further decodes the subelements of each generic element
  Authority DecodeAuthority();
  Query DecodeQuery();
  Path DecodePath();

  //Get back the generic elements of the URI
  const std::string& GetScheme() {return scheme_;}
  const std::string& GetAuthority() {return authority_;}
  const std::string& GetPath() {return path_;}
  const std::string& GetQuery() {return query_;}
  const std::string& GetFragment() {return fragment_;}
};
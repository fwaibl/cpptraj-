#include <cstdio>
#include <cctype>
#include <cstring>
#include <string>
#include <set>
#include <map>

using namespace std;

#define BUFFERSIZE 1023

/// List of strings
typedef set<string> Slist;

/// Pair string to list of strings
typedef pair<string, Slist> Spair;

/// Map string to list of strings
typedef map<string, Slist> Smap;

/// Hold dependencies for source files
Smap Sources;

/// Hold dependencies for headers
Smap Headers;

enum FileType { SOURCE = 0, HEADER };

/** Add list of dependencies for the given file to map. */
void GetDependencies(string const& filename) {
  char buffer[BUFFERSIZE+1];
  char headername[BUFFERSIZE+1];
  // Determine type
  FileType type;
  string ext;
  size_t found = filename.find_last_of(".");
  if (found != string::npos)
    ext = filename.substr(found);

  printf("FILE: %s  EXT: %s\n", filename.c_str(), ext.c_str());
  string key;
  if (ext == ".cpp" || ext == ".c") {
    type = SOURCE;
    key = filename.substr(0,found) + ".o";
    // Each source file should only be accessed once
    Smap::iterator it = Sources.find( key );
    if (it != Sources.end()) {
      fprintf(stderr,"Error: Source '%s' is being looked at more than once.\n", filename.c_str());
      return;
    }
  } else if (ext == ".h") {
    type = HEADER;
    key = filename;
    // If this header was already looked at return
    Smap::iterator it = Headers.find( key );
    if (it != Headers.end()) {
      printf("\tSkipping already-seen header %s\n", filename.c_str());
      return;
    }
  } else // Ignore all others for now
    return;
  printf("KEY: %s\n", key.c_str());

  FILE* infile = fopen(filename.c_str(), "rb");
  if (infile == 0) {
    fprintf(stderr,"Error: Could not open '%s'\n", filename.c_str());
    return;
  }
  // Go through file and grab includes.
  Slist depends;
  while ( fgets(buffer, BUFFERSIZE, infile) != 0 )
  {
    char* ptr = buffer;
    // Skip leading whitespace.
    while ( isspace(*ptr) ) {
      ptr++;
      if (*ptr=='\n' || *ptr=='\0') break;
    }
    // First non-whitespace charcter must be '#'
    if (*ptr == '#') {
      // Check if this is an include line - if not, skip
      char* includePosition = strstr(ptr,"include");
      if ( includePosition != 0 ) {
        // Skip over system headers
        if ( strchr(ptr, '<') == 0 ) {
          // Get header name - assume it is second string - dont include first "
          sscanf(includePosition, "%*s \"%s", headername);
          // Get rid of last "
          size_t pos = strlen(headername);
          if (headername[pos-1]=='"') headername[pos-1]='\0';
          depends.insert( string(headername) );
        } else
          printf("\tSkipping system header line: %s", buffer);
      }
    }
  }
  fclose( infile );
  printf("  %s depends:", filename.c_str());
  for (Slist::const_iterator it = depends.begin(); it != depends.end(); ++it)
    printf(" %s", it->c_str());
  printf("\n");
  //pair<Smap::iterator, bool> ret;
  if (type == SOURCE)
    Sources.insert( Spair(key, depends) );
  else
    Headers.insert( Spair(key, depends) );
  for (Slist::const_iterator it = depends.begin(); it != depends.end(); ++it)
     GetDependencies( *it ); 
}

// M A I N
int main(int argc, char** argv) {
  if (argc < 2) return  0;
  for (int i=1; i<argc; i++)
    GetDependencies(argv[i]);

  return 0;
}

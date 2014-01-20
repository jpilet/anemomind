
#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <string>

namespace sail
{

char getFileSep();
bool isFileSep(char c);
std::string concatPaths(std::string a, std::string b);

class Path
{
public:
	Path();
	Path(std::string p);
	Path cat(std::string p);
	std::string str();
private:
	std::string _path;
};

void splitFilename(std::string filename, std::string &outPrefix, std::string &outSuffix);
bool exists(std::string filename);
int getFileSize(std::string filename);

} /* namespace sail */
#endif /* FILESYSTEM_H_ */

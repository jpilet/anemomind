#include "filesystem.h"
#include <iostream>
#include <fstream>
#include "common.h"

namespace sail
{

char getFileSep()
{
	return '/';
}

bool isFileSep(char c)
{
	return c == '/' || c == '\\';
}

std::string concatPaths(std::string a, std::string b)
{
	if (a.empty())
	{
		return b;
	}
	else if (b.empty())
	{
		return a;
	}

	char fs = getFileSep();
	int alen = a.length();
	char alast = a[alen-1];
	char bfirst = b[0];
	int blen = b.size();
	if (isFileSep(alast))
	{
		if (isFileSep(bfirst))
		{
			return a + b.substr(1, blen-1);
		}
		else
		{
			return a + b;
		}
	}
	else
	{
		if (isFileSep(bfirst))
		{
			return a + b;
		}
		else
		{
			return a + getFileSep() + b;
		}
	}
}

Path::Path()
{}

Path::Path(std::string p)
{
	_path = p;
}

Path Path::cat(std::string p)
{
	return Path(concatPaths(_path, p));
}

std::string Path::str()
{
	return _path;
}

bool exists(std::string filename)
{
	std::ifstream myfile (filename, std::ios::binary);
	return myfile.is_open();
}

void splitFilename(std::string filename, std::string &outPrefix, std::string &outSuffix)
{
	int pos = filename.find_last_of('.');
	int len = filename.length();

//	DOUT(pos);
//	DOUT(len);
//	DOUT(filename);

	if (pos >= len)
	{
		outPrefix = filename;
		outSuffix = "";
	}
	else
	{
		outPrefix = filename.substr(0, pos);
		int pos1 = pos+1;
		outSuffix = filename.substr(pos1, len-pos1);
	}
//
//	DOUT(outPrefix);
//	DOUT(outSuffix);
}

int getFileSize(std::string filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate | std::ios::in);
	return file.tellg();
}


} /* namespace sail */

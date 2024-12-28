#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <string>
#include <vector>

using namespace std;

class FileHandler
{
public:
	static bool LoadFile(const wstring& filename, vector<unsigned char>& data);
	static bool SaveFile(const wstring& filename, const vector<unsigned char>& data);
};


#endif // FILEHANDLER_H

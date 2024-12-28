#include "FileHandler.h"
#include <fstream>

using namespace std;

bool FileHandler::LoadFile(const wstring& filename, vector<unsigned char>& data)
{
	ifstream file(filename, ios::binary);

	if (!file)
	{
		return false;
	}

	// Ermittlung der Dateigröße
	file.seekg(0, ios::end);
	streamsize size = file.tellg();
	file.seekg(0, ios::beg);

	if (size < 0)
	{
		return false;
	}

	data.resize(static_cast<size_t>(size));

	if (!file.read(reinterpret_cast<char*>(data.data()), size))
	{
		return false;
	}

	return true;
}

bool FileHandler::SaveFile(const wstring& filename, const vector<unsigned char>& data)
{
	ofstream file(filename, ios::binary);

	if (!file)
	{
		return false;
	}
	else
	{
		file.write(reinterpret_cast<const char*>(data.data()), data.size());
		return file.good();
	}
}
#ifndef HEXEDITOR_H
#define HEXEDITOR_H

#include <vector>
#include <string>

using namespace std;

class HexEditor
{
public:
	HexEditor();
	bool LoadFile(const wstring& filename);
	bool SaveFile(const wstring& filename);
	const vector<unsigned char>& GetData() const;
	void SetData(const vector<unsigned char>& data);
	unsigned char GetByte(size_t index) const;
	void SetByte(size_t index, unsigned char value);
	size_t GetSize() const;

private:
	vector<unsigned char> fileData;

};


#endif // HEXEDITOR_H

#include "HexEditor.h"
#include "FileHandler.h"
#include <vector>

using namespace std;

// Konstruktor
HexEditor::HexEditor()
{

}

bool HexEditor::LoadFile(const wstring& filename)
{
	return FileHandler::LoadFile(filename, fileData);
}

bool HexEditor::SaveFile(const wstring& filename)
{
	return FileHandler::SaveFile(filename, fileData);
}

const vector<unsigned char>& HexEditor::GetData() const
{
	return fileData;
}

void HexEditor::SetData(const vector<unsigned char>& data)
{
	fileData = data;
}

unsigned char HexEditor::GetByte(size_t index) const
{
	if (index < fileData.size())
	{
		return fileData[index];
	}

	return 0;
}

void HexEditor::SetByte(size_t index, unsigned char value)
{
	if (index < fileData.size())
	{
		fileData[index] = value;
	}
}

size_t HexEditor::GetSize() const
{
	return fileData.size();
}

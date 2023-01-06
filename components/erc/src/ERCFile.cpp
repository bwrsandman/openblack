/*******************************************************************************
 * Copyright (c) 2018-2023 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *******************************************************************************
 *
 * The layout of a ERC File is as follows:
 *
 * - 8 byte header containing:
 *        unknown 4 bytes
 *        unknown 4 bytes
 * - 2 variable size checksums containing:
 *        1 byte describing the length of the checksum
 *        checksum
 * - 1 variable size creature name containing:
 *        4 bytes describing the string length of the name
 *        variable size array of wide chars describing the name
 *
 */

#include "ERCFile.h"

#include <cassert>
#include <cstring>

#include <fstream>

using namespace openblack::erc;

namespace
{
// Adapted from https://stackoverflow.com/a/13059195/10604387
//          and https://stackoverflow.com/a/46069245/10604387
struct membuf: std::streambuf
{
	membuf(char const* base, size_t size)
	{
		char* p(const_cast<char*>(base));
		this->setg(p, p, p + size);
	}
	std::streampos seekoff(off_type off, std::ios_base::seekdir way, [[maybe_unused]] std::ios_base::openmode which) override
	{
		if (way == std::ios_base::cur)
		{
			gbump(static_cast<int>(off));
		}
		else if (way == std::ios_base::end)
		{
			setg(eback(), egptr() + off, egptr());
		}
		else if (way == std::ios_base::beg)
		{
			setg(eback(), eback() + off, egptr());
		}
		return gptr() - eback();
	}

	std::streampos seekpos([[maybe_unused]] pos_type pos, [[maybe_unused]] std::ios_base::openmode which) override
	{
		return seekoff(pos - pos_type(off_type(0)), std::ios_base::beg, which);
	}
};
struct imemstream: virtual membuf, std::istream
{
	imemstream(char const* base, size_t size)
	    : membuf(base, size)
	    , std::istream(dynamic_cast<std::streambuf*>(this))
	{
	}
};
} // namespace

/// Error handling
void ERCFile::Fail(const std::string& msg)
{
	throw std::runtime_error("ERC Error: " + msg + "\nFilename: " + _filename.string());
}

ERCFile::ERCFile() = default;
ERCFile::~ERCFile() = default;

void ERCFile::ReadFile(std::istream& stream)
{
	assert(!_isLoaded);

	// Total file size
	std::size_t fsize = 0;
	if (stream.seekg(0, std::ios_base::end))
	{
		fsize = static_cast<std::size_t>(stream.tellg());
		stream.seekg(0);
	}

	if (fsize < sizeof(ERCHeader) + sizeof(uint8_t))
	{
		Fail("File too small to be a valid ERC file.");
	}

	// First 8 bytes
	stream.read(reinterpret_cast<char*>(&_header), sizeof(_header));

	uint8_t checksumSize;
	stream.read(reinterpret_cast<char*>(&checksumSize), sizeof(checksumSize));
	_checksum1.resize(checksumSize);
	if (fsize < static_cast<std::size_t>(stream.tellg()) + _checksum1.size() * sizeof(_checksum1[0]) + sizeof(uint8_t)) {
		Fail("File too small to contain checksum.");
	}
	stream.read(reinterpret_cast<char*>(_checksum1.data()), _checksum1.size() * sizeof(_checksum1[0]));
	stream.read(reinterpret_cast<char*>(&checksumSize), sizeof(checksumSize));
	_checksum2.resize(checksumSize);
	if (fsize < static_cast<std::size_t>(stream.tellg()) + _checksum2.size() * sizeof(_checksum2[0]) + sizeof(uint32_t)) {
		Fail("File too small to contain checksum.");
	}
	stream.read(reinterpret_cast<char*>(_checksum2.data()), _checksum2.size() * sizeof(_checksum2[0]));

	uint32_t creatureNameLength;
	stream.read(reinterpret_cast<char*>(&creatureNameLength), sizeof(creatureNameLength));
	std::vector<wchar_t> creatureNameRaw;
	creatureNameRaw.resize(creatureNameLength);
	if (fsize < static_cast<std::size_t>(stream.tellg()) + creatureNameRaw.size() * sizeof(creatureNameRaw[0])) {
		Fail("File too small to contain creature name.");
	}
	stream.read(reinterpret_cast<char*>(creatureNameRaw.data()), creatureNameRaw.size() * sizeof(creatureNameRaw[0]));
	_creatureName = std::wstring(creatureNameRaw.cbegin(), creatureNameRaw.cend());

	// TODO

	_isLoaded = true;
}

void ERCFile::WriteFile([[maybe_unused]] std::ostream& stream) const
{
	assert(!_isLoaded);

	// TODO
}

void ERCFile::Open(const std::filesystem::path& filepath)
{
	assert(!_isLoaded);

	_filename = filepath;

	std::ifstream stream(_filename, std::ios::binary);

	if (!stream.is_open())
	{
		Fail("Could not open file.");
	}

	ReadFile(stream);
}

void ERCFile::Open(const std::vector<uint8_t>& buffer)
{
	assert(!_isLoaded);

	imemstream stream(reinterpret_cast<const char*>(buffer.data()), buffer.size() * sizeof(buffer[0]));

	// File name set to "buffer" when file is load from a buffer
	// Impact code using ERCFile::GetFilename method
	_filename = std::filesystem::path("buffer");

	ReadFile(stream);
}

void ERCFile::Write(const std::filesystem::path& filepath)
{
	assert(!_isLoaded);

	_filename = filepath;

	std::ofstream stream(_filename, std::ios::binary);

	if (!stream.is_open())
	{
		Fail("Could not open file.");
	}

	WriteFile(stream);
}

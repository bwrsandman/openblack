/*****************************************************************************
 * Copyright (c) 2018-2023 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include <cstdint>

#include <filesystem>
#include <string>
#include <vector>

namespace openblack::erc
{

struct ERCHeader
{
	uint32_t field_0x0;
	uint32_t field_0x4;
};

/**
  This class is used to read ERCs aka creature mind files.
 */
class ERCFile
{
protected:
	/// True when a file has been loaded
	bool _isLoaded {false};

	std::filesystem::path _filename;

public: // TODO Remove
	ERCHeader _header;
	std::vector<int8_t> _checksum1;
	std::vector<int8_t> _checksum2;
	std::wstring _creatureName;

	/// Error handling
	void Fail(const std::string& msg);

	/// Read file from the input source
	virtual void ReadFile(std::istream& stream);

	/// Write file to the input source
	virtual void WriteFile(std::ostream& stream) const;

public:
	ERCFile();
	virtual ~ERCFile();

	/// Read anm file from the filesystem
	void Open(const std::filesystem::path& filepath);

	/// Read anm file from a buffer
	void Open(const std::vector<uint8_t>& buffer);

	/// Write anm file to path on the filesystem
	void Write(const std::filesystem::path& filepath);

	[[nodiscard]] std::string GetFilename() const { return _filename.string(); }
};

} // namespace openblack::anm

/*****************************************************************************
 * Copyright (c) 2018-2022 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include <cstdlib>

#include <string>

#include <ERCFile.h>
#include <cxxopts.hpp>

struct Arguments
{
	struct Read
	{
		std::vector<std::filesystem::path> filenames;
	} read;
};


bool parseOptions(int argc, char** argv, Arguments& args, int& returnCode) noexcept
{
	cxxopts::Options options("erctool", "Inspect and extract files from LionHead ERC files.");

	try
	{
		options.add_options()                                            //
		    ("h,help", "Display this help message.")                     //
		    ("subcommand", "Subcommand.", cxxopts::value<std::string>()) //
		    ;
		options.positional_help("[read] [OPTION...]");
		options.add_options("read")                                                                                       //
			// TODO Remove
			("D,debug", "Print Everything.", cxxopts::value<std::vector<std::filesystem::path>>())                  //
			;
		options.parse_positional({"subcommand"});
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		returnCode = EXIT_FAILURE;
		return false;
	}

	try
	{
		auto result = options.parse(argc, argv);
		if (result["help"].as<bool>())
		{
			std::cout << options.help() << std::endl;
			returnCode = EXIT_SUCCESS;
			return false;
		}
		if (result["subcommand"].count() == 0)
		{
			std::cerr << options.help() << std::endl;
			returnCode = EXIT_FAILURE;
			return false;
		}
		if (result["subcommand"].as<std::string>() == "read")
		{
			// TODO Remove
			if (result["debug"].count() > 0)
			{
				args.read.filenames = result["debug"].as<std::vector<std::filesystem::path>>();
				return true;
			}
		}
	}
	catch (const std::exception& err)
	{
		std::cerr << err.what() << std::endl;
	}

	std::cerr << options.help() << std::endl;
	returnCode = EXIT_FAILURE;
	return false;
}

int main(int argc, char* argv[]) noexcept
{
	Arguments args;
	int returnCode = EXIT_SUCCESS;
	if (!parseOptions(argc, argv, args, returnCode))
	{
		return returnCode;
	}

	for (auto& filename : args.read.filenames)
	{
		openblack::erc::ERCFile erc;
		try
		{
			std::cout << "Opening " << filename << std::endl; // TODO Remove
			// Open file
			erc.Open(filename);
			printf("field_0x0: %u\n", erc._header.field_0x0);
			printf("field_0x4: %u\n", erc._header.field_0x4);
			if (!erc._creatureName.empty()) {
				printf("creature name: %ls\n", erc._creatureName.c_str());
			}
			else {
				printf("no creature name\n");
			}
		}
		catch (std::exception& err)
		{
			std::cerr << err.what() << std::endl;
			returnCode |= EXIT_FAILURE;
		}
	}

	return returnCode;
}

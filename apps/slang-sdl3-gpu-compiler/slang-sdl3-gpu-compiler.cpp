/******************************************************************************
 * Copyright (c) 2018-2026 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *******************************************************************************/

#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdlib>

#include <array>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <cxxopts.hpp>
#include <slang-com-helper.h>
#include <slang-com-ptr.h>
#include <slang.h>

using namespace std::literals;

struct Arguments
{
	std::vector<std::filesystem::path> shaderPaths;
	std::filesystem::path outDirectory;
};

bool parseOptions(int argc, char** argv, Arguments& args, int& returnCode) noexcept
{
	cxxopts::Options options("slang-sdl3-gpu-compiler",
	                         "Compile slang shaders into headers with SDL3 GPU Pipeline Configurations.");

	options.parse_positional({"source"});
	options.positional_help("SOURCE...");

	options.add_options()                                                                                           //
	    ("h,help", "Display this help message.")                                                                    //
	    ("o,output-dir", "Directory to write generated shaders to.", cxxopts::value<decltype(args.outDirectory)>()) //
	    ("source", "Source Files.", cxxopts::value<decltype(args.shaderPaths)>())                                   //
	    ;

	auto result = options.parse(argc, argv);
	if (result["help"].as<bool>())
	{
		std::cout << options.help() << '\n';
		returnCode = EXIT_SUCCESS;
		return false;
	}

	// Following this, all args require positional arguments
	if (result["source"].count() == 0)
	{
		std::cerr << "Option \"source\" is missing an argument\n" << options.help() << '\n';
		returnCode = EXIT_FAILURE;
		return false;
	}

	args.shaderPaths = result["source"].as<decltype(args.shaderPaths)>();
	args.outDirectory = result["output-dir"].as<decltype(args.outDirectory)>();

	returnCode = EXIT_SUCCESS;
	return true;
}

void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob)
{
	if (diagnosticsBlob != nullptr)
	{
		std::cerr << static_cast<const char*>(diagnosticsBlob->getBufferPointer()) << '\n';
	}
}

Slang::ComPtr<slang::ISession> createSession(Slang::ComPtr<slang::IGlobalSession> globalSession)
{
	const slang::TargetDesc targetDesc = {
	    .format = SLANG_SPIRV,
	    .profile = globalSession->findProfile("spirv_1_5"),
	};

	std::array<slang::CompilerOptionEntry, 1> options = {{{.name = slang::CompilerOptionName::EmitSpirvDirectly,
	                                                       .value = {.kind = slang::CompilerOptionValueKind::Int,
	                                                                 .intValue0 = 1,
	                                                                 .intValue1 = 0,
	                                                                 .stringValue0 = nullptr,
	                                                                 .stringValue1 = nullptr}}}};

	const slang::SessionDesc sessionDesc = {
	    .targets = &targetDesc,
	    .targetCount = 1,
	    .compilerOptionEntries = options.data(),
	    .compilerOptionEntryCount = options.size(),
	};

	Slang::ComPtr<slang::ISession> session;
	globalSession->createSession(sessionDesc, session.writeRef());

	return session;
}

struct ShaderReadResult
{
	std::string contents;
	std::error_code error;
};

ShaderReadResult readShader(const std::filesystem::path& shaderSrcPath)
{
	ShaderReadResult result {};
	std::error_code ec;

	if (!std::filesystem::exists(shaderSrcPath, ec) || ec)
	{
		result.error = ec ? ec : std::make_error_code(std::errc::no_such_file_or_directory);
		return result;
	}

	if (!std::filesystem::is_regular_file(shaderSrcPath, ec) || ec)
	{
		result.error = ec ? ec : std::make_error_code(std::errc::invalid_argument);
		return result;
	}

	std::ifstream in(shaderSrcPath, std::ios::binary);
	if (!in.is_open())
	{
		result.error = std::make_error_code(std::errc::io_error);
		return result;
	}

	const auto size = std::filesystem::file_size(shaderSrcPath, ec);
	if (!ec)
	{
		result.contents.resize(static_cast<std::size_t>(size));
		in.read(result.contents.data(), result.contents.size());
		if (!in)
		{
			result.error = std::make_error_code(std::errc::io_error);
		}
	}
	else
	{
		result.contents.assign(std::istreambuf_iterator<char>(in), {});
		if (!in.good() && !in.eof())
		{
			result.error = std::make_error_code(std::errc::io_error);
		}
	}

	return result;
}

void hexDump(std::ofstream& out, const std::string& name, std::span<const uint8_t> binaryData)
{
	constexpr uint32_t numBytesPerLine = 0x10;
	constexpr uint32_t spaces = 4;

	std::println(out, "constexpr std::array<uint8_t, 0x{:x}> {} = {{", binaryData.size_bytes(), name);
	for (auto chunk : binaryData | std::views::chunk(numBytesPerLine))
	{
		std::print(out, "{: <{}}", "", spaces - 1);
		for (auto b : chunk)
		{
			std::print(out, " 0x{:02x},", b);
		}
		std::println(out, "");
	}
	std::println(out, "}};");
}

void writeGPUShaderCreateInfo(std::ofstream& out, const std::string& name, const std::string& codeVarName,
                              const std::string& entrypoint, SlangCompileTarget compileTarget, SlangStage stage,
                              uint32_t numSamplers, uint32_t numStorageTextures, uint32_t numStorageBuffers,
                              uint32_t numUniformBuffers)
{
	constexpr uint32_t spaces = 4;

	assert(compileTarget == SlangCompileTarget::SLANG_SPIRV || compileTarget == SlangCompileTarget::SLANG_DXIL ||
	       compileTarget == SlangCompileTarget::SLANG_METAL);
	assert(stage == SlangStage::SLANG_STAGE_VERTEX || stage == SlangStage::SLANG_STAGE_FRAGMENT);

	std::println(out, "constexpr auto {} = SDL_GPUShaderCreateInfo {{", name);
	std::println(out, "{: <{}}.code_size = {}.size(),", "", spaces, codeVarName);
	std::println(out, "{: <{}}.code = {}.data(),", "", spaces, codeVarName);
	std::println(out, "{: <{}}.entrypoint = \"{}\",", "", spaces, entrypoint);
	std::println(
	    out, "{: <{}}.format = {},", "", spaces,
	    compileTarget == SlangCompileTarget::SLANG_SPIRV
	        ? "SDL_GPU_SHADERFORMAT_SPIRV"
	        : (compileTarget == SlangCompileTarget::SLANG_DXIL ? "SDL_GPU_SHADERFORMAT_DXIL" : "SDL_GPU_SHADERFORMAT_MSL"));
	std::println(out, "{: <{}}.stage = {},", "", spaces,
	             stage == SlangStage::SLANG_STAGE_VERTEX ? "SDL_GPU_SHADERSTAGE_VERTEX" : "SDL_GPU_SHADERSTAGE_FRAGMENT");
	std::println(out, "{: <{}}.num_samplers = {},", "", spaces, numSamplers);
	std::println(out, "{: <{}}.num_storage_textures = {},", "", spaces, numStorageTextures);
	std::println(out, "{: <{}}.num_storage_buffers = {},", "", spaces, numStorageBuffers);
	std::println(out, "{: <{}}.num_uniform_buffers = {},", "", spaces, numUniformBuffers);
	std::println(out, "{: <{}}.props = 0,", "", spaces);
	std::println(out, "}};");
}

std::string toString(slang::TypeReflection::ScalarType type)
{
	switch (type)
	{
	case slang::TypeReflection::ScalarType::Int32:
		return "INT";
	case slang::TypeReflection::ScalarType::UInt32:
		return "UINT";
	case slang::TypeReflection::ScalarType::Float16:
		return "HALF";
	case slang::TypeReflection::ScalarType::Float32:
		return "FLOAT";
	case slang::TypeReflection::ScalarType::Int8:
		return "BYTE";
	case slang::TypeReflection::ScalarType::UInt8:
		return "UBYTE";
	case slang::TypeReflection::ScalarType::Int16:
		return "SHORT";
	case slang::TypeReflection::ScalarType::UInt16:
		return "USHORT";
	default:
		assert(false);
		std::unreachable();
	}
}

std::string toString(slang::TypeReflection* refl)
{
	switch (refl->getKind())
	{
	case slang::TypeReflection::Kind::Scalar:
	case slang::TypeReflection::Kind::Vector:
		return "SDL_GPU_VERTEXELEMENTFORMAT_"s + toString(refl->getScalarType()) + (refl->getColumnCount() > 1 ? std::to_string(refl->getColumnCount()) : "");
	default:
		assert(false);
		std::unreachable();
	}
}

uint32_t getSize(slang::TypeReflection::ScalarType type)
{
	switch (type)
	{
	case slang::TypeReflection::ScalarType::Int32:
	case slang::TypeReflection::ScalarType::UInt32:
	case slang::TypeReflection::ScalarType::Float32:
		return 4;
	case slang::TypeReflection::ScalarType::Int16:
	case slang::TypeReflection::ScalarType::UInt16:
	case slang::TypeReflection::ScalarType::Float16:
		return 2;
	case slang::TypeReflection::ScalarType::Int8:
	case slang::TypeReflection::ScalarType::UInt8:
		return 1;
	default:
		assert(false);
		std::unreachable();
	}
}

uint32_t getSize(slang::TypeReflection* refl)
{
	switch (refl->getKind())
	{
	case slang::TypeReflection::Kind::Scalar:
	case slang::TypeReflection::Kind::Vector:
		return getSize(refl->getScalarType()) * refl->getColumnCount();
	default:
		assert(false);
		std::unreachable();
	}
}

void writeGPUVertexAttributes(std::ofstream& out, const std::string& name, std::span<std::string> types,
                              std::span<uint32_t> sizes)
{
	constexpr uint32_t spaces = 4;
	uint32_t currentOffset = 0;
	std::println(out, "constexpr auto {} = std::array {{", name);
	for (const auto&& [location, type, size] : std::views::zip(std::views::iota(0), types, sizes))
	{
		uint32_t bufferSlot = 0;
		std::println(out, "{: <{}}SDL_GPUVertexAttribute {{", "", spaces);
		std::println(out, "{: <{}}.location = {},", "", spaces * 2, location);
		std::println(out, "{: <{}}.buffer_slot = {},", "", spaces * 2, bufferSlot);
		std::println(out, "{: <{}}.format = {},", "", spaces * 2, type);
		std::println(out, "{: <{}}.offset = {},", "", spaces * 2, currentOffset);
		std::println(out, "{: <{}}}},", "", spaces);
		currentOffset += size;
	}
	std::println(out, "}};");
}

void writeGPUVertexBufferDescription(std::ofstream& out, const std::string& name, uint32_t pitch, bool instanced)
{
	constexpr uint32_t spaces = 4;
    const uint32_t slot = 0;
	std::println(out, "constexpr auto {} = SDL_GPUVertexBufferDescription {{", name + (instanced ? "ByInstance" : "ByVertex"));
	std::println(out, "{: <{}}.slot = {},", "", spaces, slot);
	std::println(out, "{: <{}}.pitch = {},", "", spaces, pitch);
	std::println(out, "{: <{}}.input_rate = {},", "", spaces, instanced ? "SDL_GPU_VERTEXINPUTRATE_INSTANCE" : "SDL_GPU_VERTEXINPUTRATE_VERTEX");
	std::println(out, "{: <{}}.instance_step_rate = 0,", "", spaces);
	std::println(out, "}};");
}

void writeGPUVertexInputState(std::ofstream& out, const std::string& name, bool instanced)
{
	constexpr uint32_t spaces = 4;
    const auto attributes = name + "VertexAttributes";
    const auto descriptions = name + "VertexBufferDescription" + (instanced ? "ByInstance" : "ByVertex");
	std::println(out, "constexpr auto {} = SDL_GPUVertexInputState {{", name + "VertexInputState" + (instanced ? "ByInstance" : "ByVertex"));
	std::println(out, "{: <{}}.vertex_buffer_descriptions = &{},", "", spaces, descriptions);
	std::println(out, "{: <{}}.num_vertex_buffers = 1,", "", spaces);
	std::println(out, "{: <{}}.vertex_attributes = {}.data(),", "", spaces, attributes);
	std::println(out, "{: <{}}.num_vertex_attributes = {}.size(),", "", spaces, attributes);
	std::println(out, "}};");
}

constexpr std::array<std::string_view, SLANG_STAGE_COUNT> k_StageReadableStrings = std::array {
    "None"sv,         "Vertex"sv, "Hull"sv,       "Domain"sv, "Geometry"sv, "Fragment"sv, "Compute"sv,       "RayGeneration"sv,
    "Intersection"sv, "AnyHit"sv, "ClosestHit"sv, "Miss"sv,   "Callable"sv, "Mesh"sv,     "Amplification"sv, "Dispatch"sv,
};

void writeHeader(const std::filesystem::path& headerPath, SlangCompileTarget target, const std::string& shaderName,
                 const std::vector<Slang::ComPtr<slang::IBlob>>& spirvCodeCollection,
                 const std::vector<slang::ProgramLayout*>& layouts)
{
	std::ofstream out(headerPath);

	// Includes
	std::println(out, "#include <array>");
	std::println(out, "");
	std::println(out, "#include <SDL3/SDL_gpu.h>");

	// Compiled shader data
	for (const auto& [spirvCode, layout] : std::views::zip(spirvCodeCollection, layouts))
	{
		assert(layout->getEntryPointCount() == 1);

		auto* entry = layout->getEntryPointByIndex(0);
		auto stage = entry->getStage();

		const auto stagePrefix =
		    "k_"s + static_cast<char>(std::toupper(shaderName[0])) + &shaderName[1] + k_StageReadableStrings[stage].data();
		std::println(out, "");
		const auto codeVarName = stagePrefix + "Bytes";
		hexDump(out, codeVarName,
		        {reinterpret_cast<const uint8_t*>(spirvCode->getBufferPointer()), spirvCode->getBufferSize()});
	}

	// SDL_GPUShaderCreateInfo
	for (const auto& layout : layouts)
	{
		auto* entry = layout->getEntryPointByIndex(0);
		auto stage = entry->getStage();

		uint32_t numSamplers = 0;
		uint32_t numStorageTextures = 0;
		uint32_t numStorageBuffers = 0;
		uint32_t numUniformBuffers = 0;
		for (uint32_t i = 0; i < layout->getParameterCount(); ++i)
		{
			auto* parameter = layout->getParameterByIndex(i);
			if (stage == SlangStage::SLANG_STAGE_VERTEX)
			{
				switch (parameter->getBindingSpace())
				{
				case 0:
					// assert(category == slang::ParameterCategory::SamplerState);
					++numSamplers;
					break;
				case 1:
					// assert(category == slang::ParameterCategory::Uniform);
					++numUniformBuffers;
					break;
				}
			}
			else if (stage == SlangStage::SLANG_STAGE_FRAGMENT)
			{
				switch (parameter->getBindingSpace())
				{
				case 2:
					// assert(category == slang::ParameterCategory::SamplerState);
					++numSamplers;
					break;
				case 3:
					// assert(category == slang::ParameterCategory::Uniform);
					++numUniformBuffers;
					break;
				}
			}
			else
			{
				assert(false);
				std::unreachable();
			}
		}

		const auto stagePrefix =
		    "k_"s + static_cast<char>(std::toupper(shaderName[0])) + &shaderName[1] + k_StageReadableStrings[stage].data();
		const auto codeVarName = stagePrefix + "Bytes";
		std::println(out, "");
		const auto shaderCreateInfoVarName = stagePrefix + "ShaderCreateInfo";
		writeGPUShaderCreateInfo(out, shaderCreateInfoVarName, codeVarName, "main", target, stage, numSamplers,
		                         numStorageTextures, numStorageBuffers, numUniformBuffers);
	}

	// SDL_GPUVertexAttribute
	for (const auto& layout : layouts)
	{
		auto* entry = layout->getEntryPointByIndex(0);
		auto stage = entry->getStage();

		if (stage == SlangStage::SLANG_STAGE_VERTEX)
		{
			std::vector<std::string> types;
			std::vector<uint32_t> sizes;
			for (uint32_t i = 0; i < entry->getParameterCount(); ++i)
			{
				auto* param = entry->getParameterByIndex(i);
				auto* varLayout = param->getVariable();
				auto* typeLayout = varLayout->getType();
				assert(typeLayout->getKind() == slang::TypeReflection::Kind::Struct);
				for (uint32_t i = 0; i < typeLayout->getFieldCount(); ++i)
				{
					auto* field = typeLayout->getFieldByIndex(i);
					auto* fieldType = field->getType();
					types.emplace_back(toString(fieldType));
					sizes.emplace_back(getSize(fieldType));
				}
			}

			std::println(out, "");

			const auto name = "k_"s + static_cast<char>(std::toupper(shaderName[0])) + &shaderName[1] + "VertexAttributes";
			writeGPUVertexAttributes(out, name, types, sizes);
		}
	}

	// SDL_GPUVertexBufferDescription, both vertex and TODO: instance
	for (const auto& layout : layouts)
	{
		auto* entry = layout->getEntryPointByIndex(0);
		auto stage = entry->getStage();

		if (stage == SlangStage::SLANG_STAGE_VERTEX)
		{
			uint32_t vertexPitch = 0;
			for (uint32_t i = 0; i < entry->getParameterCount(); ++i)
			{
				auto* param = entry->getParameterByIndex(i);
				auto* varLayout = param->getVariable();
				auto* typeLayout = varLayout->getType();
				assert(typeLayout->getKind() == slang::TypeReflection::Kind::Struct);
				for (uint32_t i = 0; i < typeLayout->getFieldCount(); ++i)
				{
					auto* field = typeLayout->getFieldByIndex(i);
					vertexPitch += getSize(field->getType());
				}
			}

			std::println(out, "");

			const auto name = "k_"s + static_cast<char>(std::toupper(shaderName[0])) + &shaderName[1] + "VertexBufferDescription";
			writeGPUVertexBufferDescription(out, name, vertexPitch, false);
			writeGPUVertexBufferDescription(out, name, vertexPitch, true);
		}
	}

	// SDL_GPUVertexInputState
	for (const auto& layout : layouts)
	{
        auto* entry = layout->getEntryPointByIndex(0);
		auto stage = entry->getStage();

		if (stage == SlangStage::SLANG_STAGE_VERTEX)
		{
			std::println(out, "");

			const auto name = "k_"s + static_cast<char>(std::toupper(shaderName[0])) + &shaderName[1];
			writeGPUVertexInputState(out, name, false);
			writeGPUVertexInputState(out, name, true);
        }
	}
}

int main(int argc, char* argv[]) noexcept
{
	Arguments args;
	int returnCode = EXIT_SUCCESS;
	if (!parseOptions(argc, argv, args, returnCode))
	{
		return returnCode;
	}

	Slang::ComPtr<slang::IGlobalSession> globalSession;
	createGlobalSession(globalSession.writeRef());

	auto session = createSession(globalSession);

	for (const auto& shaderPath : args.shaderPaths)
	{
		auto shaderReadResult = readShader(shaderPath);
		const auto shaderName = shaderPath.stem();

		if (shaderReadResult.error)
		{
			std::cerr << "Failed to read shader \"" << shaderPath << "\": " << shaderReadResult.error.message()
			          << " (category=" << shaderReadResult.error.category().name() << ")\n";

			return EXIT_FAILURE;
		}

		Slang::ComPtr<slang::IModule> slangModule;
		{
			Slang::ComPtr<slang::IBlob> diagnosticsBlob;
			slangModule = session->loadModuleFromSourceString(shaderName.c_str(), shaderPath.filename().c_str(),
			                                                  shaderReadResult.contents.c_str(), diagnosticsBlob.writeRef());
			diagnoseIfNeeded(diagnosticsBlob);
			if (slangModule == nullptr)
			{
				return EXIT_FAILURE;
			}
		}

		const auto numEntryPoints = slangModule->getDefinedEntryPointCount();

		std::vector<Slang::ComPtr<slang::IEntryPoint>> entryPoints;
		entryPoints.resize(numEntryPoints);
		for (auto i = 0; i < numEntryPoints; ++i)
		{
			Slang::ComPtr<slang::IBlob> diagnosticsBlob;
			SlangResult res = slangModule->getDefinedEntryPoint(i, entryPoints[i].writeRef());
			if (SLANG_FAILED(res))
			{
				std::cout << "Error getting entry point\n";
				return EXIT_FAILURE;
			}
		}

		std::vector<Slang::ComPtr<slang::IComponentType>> composedPrograms;
		composedPrograms.reserve(entryPoints.size());
		for (const auto& entryPoint : entryPoints)
		{
			auto componentTypes = std::array {
			    reinterpret_cast<slang::IComponentType*>(slangModule.get()),
			    reinterpret_cast<slang::IComponentType*>(entryPoint.get()),
			};
			Slang::ComPtr<slang::IBlob> diagnosticsBlob;
			Slang::ComPtr<slang::IComponentType> composedProgram;
			SlangResult result = session->createCompositeComponentType(componentTypes.data(), componentTypes.size(),
			                                                           composedProgram.writeRef(), diagnosticsBlob.writeRef());
			diagnoseIfNeeded(diagnosticsBlob);
			if (SLANG_FAILED(result))
			{
				std::cout << "Error compositing Modules and Entry Points\n";
				return EXIT_FAILURE;
			}
			composedPrograms.emplace_back(composedProgram);
		};

		std::vector<Slang::ComPtr<slang::IComponentType>> linkedPrograms;
		linkedPrograms.reserve(entryPoints.size());
		for (const auto& composedProgram : composedPrograms)
		{
			Slang::ComPtr<slang::IComponentType> linkedProgram;
			Slang::ComPtr<slang::IBlob> diagnosticsBlob;
			SlangResult result = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
			diagnoseIfNeeded(diagnosticsBlob);
			if (SLANG_FAILED(result))
			{
				std::cout << "Error Linking Programs\n";
				return EXIT_FAILURE;
			}
			linkedPrograms.emplace_back(linkedProgram);
		}

		std::vector<slang::ProgramLayout*> programLayouts;
		programLayouts.reserve(linkedPrograms.size());
		std::vector<Slang::ComPtr<slang::IBlob>> spirvCodeCollection;
		spirvCodeCollection.reserve(linkedPrograms.size());
		for (const auto& linkedProgram : linkedPrograms)
		{
			Slang::ComPtr<slang::IBlob> spirvCode;
			Slang::ComPtr<slang::IBlob> diagnosticsBlob;
			SlangResult result = linkedProgram->getEntryPointCode(0, 0, spirvCode.writeRef(), diagnosticsBlob.writeRef());
			diagnoseIfNeeded(diagnosticsBlob);
			if (SLANG_FAILED(result))
			{
				std::cout << "Error Getting Program code\n";
				return EXIT_FAILURE;
			}
			spirvCodeCollection.emplace_back(spirvCode);

			programLayouts.emplace_back(linkedProgram->getLayout());
		}

		writeHeader(args.outDirectory / (shaderPath.filename().string() + ".spv.h"), SlangCompileTarget::SLANG_SPIRV,
		            shaderName, spirvCodeCollection, programLayouts);
	}

	return returnCode;
}
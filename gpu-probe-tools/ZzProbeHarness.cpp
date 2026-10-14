#include "prx/libSceAgcDriver/Execution/include/VulkanDevice.hpp"
#include "prx/libSceAgcDriver/Graphics/include/Draw.hpp"
#include "Recompiler.hpp"
#include "VulkanTestDevice.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <span>
#include <cstdio>
#include <fstream>
#include <optional>
#include <iostream>
#include <string>
#include <vector>

namespace {

using ShaderRecompiler::ShaderStage;

constexpr std::uint32_t Threads = 64;
alignas(256) std::array<std::uint32_t, Threads * 4> Input{};
alignas(256) std::array<std::uint32_t, Threads * 16> Output{};

std::array<std::uint32_t, 4> BufferDescriptor(const void* data, std::uint32_t count) {
    const auto address = reinterpret_cast<std::uintptr_t>(data);
    return {static_cast<std::uint32_t>(address), static_cast<std::uint32_t>((address >> 32u) & 0xffffu) | (4u << 16u), count, 0x11016facu};
}

std::vector<std::uint32_t> ReadWords(const char* path) {
    std::ifstream file(path, std::ios::binary);
    std::vector<char> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::vector<std::uint32_t> words(bytes.size() / 4);
    std::memcpy(words.data(), bytes.data(), words.size() * 4);
    return words;
}

}

int main(int argc, char** argv) {
    if (argc < 3) return VulkanTestSkipped;
    try {
        const auto device = OpenVulkanTestDevice();
        if (!device) return VulkanTestSkipped;
        const auto job = ReadWords(argv[1]);
        std::size_t at = 0;
        const auto take = [&] { return job.at(at++); };
        const std::uint32_t codeCount = take();
        alignas(256) static std::vector<std::uint32_t> code;
        code.assign(job.begin() + at, job.begin() + at + codeCount);
        at += codeCount;
        const std::uint32_t modeCount = take();
        std::vector<std::array<std::uint32_t, 4>> modes(modeCount);
        for (auto& mode : modes) for (auto& value : mode) value = take();
        const std::uint32_t rowCount = take();
        const std::uint32_t* rows = job.data() + at;
        std::vector<std::uint32_t> results;
        results.reserve(static_cast<std::size_t>(modeCount) * rowCount * 16u);
        std::vector<std::uint32_t> userData(8, 0u);
        const auto input = BufferDescriptor(Input.data(), static_cast<std::uint32_t>(Input.size()));
        const auto output = BufferDescriptor(Output.data(), static_cast<std::uint32_t>(Output.size()));
        std::copy(input.begin(), input.end(), userData.begin());
        std::copy(output.begin(), output.end(), userData.begin() + 4);
        const std::span<const std::uint32_t> span(code);
        const std::array<ShaderRecompiler::MemoryRegion, 1> memory{{{reinterpret_cast<std::uintptr_t>(code.data()), std::as_bytes(span)}}};
        const ShaderRecompiler::ShaderComputeStageInfo compute{{Threads, 1, 1}, 0, {false, false, false}, false, 1};
        for (const auto& mode : modes) {
            ShaderRecompiler::RecompileRequest request{
                {ShaderStage::Compute, reinterpret_cast<std::uintptr_t>(code.data()), span, 0, {}},
                {32, 0, userData, compute, std::nullopt, std::nullopt, memory},
                device->Target(),
                {0, 0, 0, 128}
            };
            request.context.floatMode = ShaderRecompiler::ShaderFloatMode{mode[0], mode[1] != 0u, mode[2] != 0u, mode[3] != 0u};
            request.useCache = false;
            std::optional<decltype(ShaderRecompiler::Recompile(request))> compiled;
            try {
                compiled.emplace(ShaderRecompiler::Recompile(request));
            } catch (const std::exception& error) {
                std::cerr << "mode " << mode[0] << ": " << error.what() << '\n';
            }
            for (std::uint32_t start = 0; start < rowCount; start += Threads) {
                const std::uint32_t count = std::min(Threads, rowCount - start);
                Input.fill(0u);
                std::memcpy(Input.data(), rows + static_cast<std::size_t>(start) * 4u, count * 16u);
                Output.fill(compiled ? 0xdeadbeefu : 0xbadc0deu);
                if (compiled) {
                    device->Dispatch(*compiled, 1, 1, 1, {}, reinterpret_cast<std::uintptr_t>(code.data()));
                    device->WaitIdle();
                }
                results.insert(results.end(), Output.begin(), Output.begin() + count * 16u);
            }
        }
        std::ofstream out(argv[2], std::ios::binary);
        out.write(reinterpret_cast<const char*>(results.data()), static_cast<std::streamsize>(results.size() * 4u));
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

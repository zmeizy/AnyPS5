#include "prx/libSceAgcDriver/Execution/include/VulkanDevice.hpp"
#include "prx/libSceAgcDriver/Graphics/include/Draw.hpp"
#include "Recompiler.hpp"
#include "VulkanTestDevice.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <span>
#include <string>
#include <vector>

namespace {

using AgcDriver::Graphics::Require;
using ShaderRecompiler::ShaderStage;

constexpr std::uint32_t Threads = 32;
constexpr std::uint32_t Results = 16;
constexpr std::uint32_t Forms = 14;
constexpr const char* Selects[] = {"BYTE_0", "BYTE_1", "BYTE_2", "BYTE_3", "WORD_0", "WORD_1", "DWORD"};
alignas(256) std::array<std::uint32_t, Threads * 4> Input{};
alignas(256) std::array<std::uint32_t, Threads * Results> Output{};

struct alignas(256) Shader {
    std::array<std::uint32_t, 62> words;
};

const Shader Shaders[] = {
    {{{
        0x34020084u, 0x34040086u, 0xe0301000u, 0x80000401u, 0xbf8c3f70u, 0x7e1422f9u, 0x00000604u, 0x7e1622f9u,
        0x00080604u, 0x7e1822f9u, 0x00010604u, 0x7e1a22f9u, 0x00090604u, 0x7e1c22f9u, 0x00020604u, 0x7e1e22f9u,
        0x000a0604u, 0x7e2022f9u, 0x00030604u, 0x7e2222f9u, 0x000b0604u, 0x7e2422f9u, 0x00040604u, 0x7e2622f9u,
        0x000c0604u, 0x7e2822f9u, 0x00050604u, 0x7e2a22f9u, 0x000d0604u, 0x7e2c22f9u, 0x00060604u, 0x7e2e22f9u,
        0x000e0604u, 0xe0701000u, 0x80010a02u, 0xe0701004u, 0x80010b02u, 0xe0701008u, 0x80010c02u, 0xe070100cu,
        0x80010d02u, 0xe0701010u, 0x80010e02u, 0xe0701014u, 0x80010f02u, 0xe0701018u, 0x80011002u, 0xe070101cu,
        0x80011102u, 0xe0701020u, 0x80011202u, 0xe0701024u, 0x80011302u, 0xe0701028u, 0x80011402u, 0xe070102cu,
        0x80011502u, 0xe0701030u, 0x80011602u, 0xe0701034u, 0x80011702u, 0xbf810000u,
    }}},
    {{{
        0x34020084u, 0x34040086u, 0xe0301000u, 0x80000401u, 0xbf8c3f70u, 0x7e1424f9u, 0x00000604u, 0x7e1624f9u,
        0x00080604u, 0x7e1824f9u, 0x00010604u, 0x7e1a24f9u, 0x00090604u, 0x7e1c24f9u, 0x00020604u, 0x7e1e24f9u,
        0x000a0604u, 0x7e2024f9u, 0x00030604u, 0x7e2224f9u, 0x000b0604u, 0x7e2424f9u, 0x00040604u, 0x7e2624f9u,
        0x000c0604u, 0x7e2824f9u, 0x00050604u, 0x7e2a24f9u, 0x000d0604u, 0x7e2c24f9u, 0x00060604u, 0x7e2e24f9u,
        0x000e0604u, 0xe0701000u, 0x80010a02u, 0xe0701004u, 0x80010b02u, 0xe0701008u, 0x80010c02u, 0xe070100cu,
        0x80010d02u, 0xe0701010u, 0x80010e02u, 0xe0701014u, 0x80010f02u, 0xe0701018u, 0x80011002u, 0xe070101cu,
        0x80011102u, 0xe0701020u, 0x80011202u, 0xe0701024u, 0x80011302u, 0xe0701028u, 0x80011402u, 0xe070102cu,
        0x80011502u, 0xe0701030u, 0x80011602u, 0xe0701034u, 0x80011702u, 0xbf810000u,
    }}},
    {{{
        0x34020084u, 0x34040086u, 0xe0301000u, 0x80000401u, 0xbf8c3f70u, 0x7e1426f9u, 0x00000604u, 0x7e1626f9u,
        0x00080604u, 0x7e1826f9u, 0x00010604u, 0x7e1a26f9u, 0x00090604u, 0x7e1c26f9u, 0x00020604u, 0x7e1e26f9u,
        0x000a0604u, 0x7e2026f9u, 0x00030604u, 0x7e2226f9u, 0x000b0604u, 0x7e2426f9u, 0x00040604u, 0x7e2626f9u,
        0x000c0604u, 0x7e2826f9u, 0x00050604u, 0x7e2a26f9u, 0x000d0604u, 0x7e2c26f9u, 0x00060604u, 0x7e2e26f9u,
        0x000e0604u, 0xe0701000u, 0x80010a02u, 0xe0701004u, 0x80010b02u, 0xe0701008u, 0x80010c02u, 0xe070100cu,
        0x80010d02u, 0xe0701010u, 0x80010e02u, 0xe0701014u, 0x80010f02u, 0xe0701018u, 0x80011002u, 0xe070101cu,
        0x80011102u, 0xe0701020u, 0x80011202u, 0xe0701024u, 0x80011302u, 0xe0701028u, 0x80011402u, 0xe070102cu,
        0x80011502u, 0xe0701030u, 0x80011602u, 0xe0701034u, 0x80011702u, 0xbf810000u,
    }}},
    {{{
        0x34020084u, 0x34040086u, 0xe0301000u, 0x80000401u, 0xbf8c3f70u, 0x7e1428f9u, 0x00000604u, 0x7e1628f9u,
        0x00080604u, 0x7e1828f9u, 0x00010604u, 0x7e1a28f9u, 0x00090604u, 0x7e1c28f9u, 0x00020604u, 0x7e1e28f9u,
        0x000a0604u, 0x7e2028f9u, 0x00030604u, 0x7e2228f9u, 0x000b0604u, 0x7e2428f9u, 0x00040604u, 0x7e2628f9u,
        0x000c0604u, 0x7e2828f9u, 0x00050604u, 0x7e2a28f9u, 0x000d0604u, 0x7e2c28f9u, 0x00060604u, 0x7e2e28f9u,
        0x000e0604u, 0xe0701000u, 0x80010a02u, 0xe0701004u, 0x80010b02u, 0xe0701008u, 0x80010c02u, 0xe070100cu,
        0x80010d02u, 0xe0701010u, 0x80010e02u, 0xe0701014u, 0x80010f02u, 0xe0701018u, 0x80011002u, 0xe070101cu,
        0x80011102u, 0xe0701020u, 0x80011202u, 0xe0701024u, 0x80011302u, 0xe0701028u, 0x80011402u, 0xe070102cu,
        0x80011502u, 0xe0701030u, 0x80011602u, 0xe0701034u, 0x80011702u, 0xbf810000u,
    }}},
};

constexpr std::uint32_t Sources[] = {
    0x00000000u, 0xffffffffu, 0x80c1a2b3u, 0x7f6e5d4cu, 0x01020304u, 0x80808080u, 0x7f7f7f7fu, 0xff00ff00u, 0x00ff00ffu, 0x8000ffffu, 0x00800080u, 0xfedcba98u, 0x12345678u, 0xa5a5a5a5u, 0x5a5a5a5au, 0x807f807fu
};

constexpr std::array<std::array<std::array<std::uint8_t, Forms>, std::size(Sources)>, 4> Bytes{{
    {{
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
        {179, 179, 162, 162, 193, 193, 128, 128, 179, 179, 193, 193, 179, 179},
        {76, 76, 93, 93, 110, 110, 127, 127, 76, 76, 110, 110, 76, 76},
        {4, 4, 3, 3, 2, 2, 1, 1, 4, 4, 2, 2, 4, 4},
        {128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128},
        {127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127},
        {0, 0, 255, 255, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0},
        {255, 255, 0, 0, 255, 255, 0, 0, 255, 255, 255, 255, 255, 255},
        {255, 255, 255, 255, 0, 0, 128, 128, 255, 255, 0, 0, 255, 255},
        {128, 128, 0, 0, 128, 128, 0, 0, 128, 128, 128, 128, 128, 128},
        {152, 152, 186, 186, 220, 220, 254, 254, 152, 152, 220, 220, 152, 152},
        {120, 120, 86, 86, 52, 52, 18, 18, 120, 120, 52, 52, 120, 120},
        {165, 165, 165, 165, 165, 165, 165, 165, 165, 165, 165, 165, 165, 165},
        {90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90},
        {127, 127, 128, 128, 127, 127, 128, 128, 127, 127, 127, 127, 127, 127},
    }},
    {{
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 255, 0, 255, 0, 255, 0, 255, 255, 255, 255, 255, 255, 255},
        {0, 255, 0, 255, 0, 255, 0, 255, 162, 162, 128, 128, 162, 162},
        {0, 0, 0, 0, 0, 0, 0, 0, 93, 93, 127, 127, 93, 93},
        {0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 1, 1, 3, 3},
        {0, 255, 0, 255, 0, 255, 0, 255, 128, 128, 128, 128, 128, 128},
        {0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 127, 127},
        {0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255},
        {0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 255, 0, 255, 0, 0, 0, 255, 255, 255, 128, 128, 255, 255},
        {0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 255, 0, 255, 0, 255, 0, 255, 186, 186, 254, 254, 186, 186},
        {0, 0, 0, 0, 0, 0, 0, 0, 86, 86, 18, 18, 86, 86},
        {0, 255, 0, 255, 0, 255, 0, 255, 165, 165, 165, 165, 165, 165},
        {0, 0, 0, 0, 0, 0, 0, 0, 90, 90, 90, 90, 90, 90},
        {0, 0, 0, 255, 0, 0, 0, 255, 128, 128, 128, 128, 128, 128},
    }},
    {{
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 255, 255},
        {0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 193, 193},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 110, 110},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2},
        {0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 128, 128},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127},
        {0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0},
        {0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 255},
        {0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0},
        {0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 128, 128},
        {0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 220, 220},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 52},
        {0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 165, 165},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 90, 90},
        {0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 127, 127},
    }},
    {{
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 255, 255},
        {0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 128, 128},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
        {0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 128, 128},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127},
        {0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 255, 255},
        {0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 128, 128},
        {0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 254, 254},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18},
        {0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 165, 165},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 90, 90},
        {0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 128, 128},
    }},
}};

std::array<std::uint32_t, 4> BufferDescriptor(const void* data, std::uint32_t bytes) {
    const auto address = reinterpret_cast<std::uintptr_t>(data);
    return {static_cast<std::uint32_t>(address), static_cast<std::uint32_t>((address >> 32u) & 0xffffu), bytes, 0x31016facu};
}

std::string Hex(std::uint32_t value) {
    char text[16];
    std::snprintf(text, sizeof(text), "0x%08x", value);
    return text;
}

void Run(AgcDriver::VulkanDevice& device, const Shader& shader) {
    Input.fill(0u);
    for (std::uint32_t row = 0; row < std::size(Sources); ++row) {
        Input[row * 4u] = Sources[row];
    }
    Output.fill(0xdeadbeefu);
    std::vector<std::uint32_t> userData(8, 0u);
    const auto input = BufferDescriptor(Input.data(), static_cast<std::uint32_t>(Input.size() * 4u));
    const auto output = BufferDescriptor(Output.data(), static_cast<std::uint32_t>(Output.size() * 4u));
    std::copy(input.begin(), input.end(), userData.begin());
    std::copy(output.begin(), output.end(), userData.begin() + 4);
    const std::span<const std::uint32_t> code(shader.words);
    const std::array<ShaderRecompiler::MemoryRegion, 1> memory{{{reinterpret_cast<std::uintptr_t>(code.data()), std::as_bytes(code)}}};
    const ShaderRecompiler::ShaderComputeStageInfo compute{{Threads, 1, 1}, 0u, {false, false, false}, false, 1};
    ShaderRecompiler::RecompileRequest request{
        {ShaderStage::Compute, reinterpret_cast<std::uintptr_t>(code.data()), code, 0, {}},
        {32, 0, userData, compute, std::nullopt, std::nullopt, memory},
        device.Target(),
        {0, 0, 0, 128}
    };
    request.useCache = false;
    const auto result = ShaderRecompiler::Recompile(request);
    device.Dispatch(result, 1, 1, 1, {}, reinterpret_cast<std::uintptr_t>(code.data()));
    device.WaitIdle();
}

void Check(std::uint32_t byte) {
    for (std::uint32_t row = 0; row < std::size(Sources); ++row) {
        for (std::uint32_t form = 0; form < Forms; ++form) {
            const auto actual = Output[row * Results + form];
            const auto expected = std::bit_cast<std::uint32_t>(static_cast<float>(Bytes[byte][row][form]));
            Require(actual == expected, "v_cvt_f32_ubyte" + std::to_string(byte) + " src0_sel:" + Selects[form / 2u] + (form % 2u ? " sext" : "") + " source " +
                                            Hex(Sources[row]) + " is " + Hex(actual) + ", expected " + Hex(expected));
        }
    }
}

}

int main() {
    try {
        const auto device = OpenVulkanTestDevice();
        if (!device) return VulkanTestSkipped;
        for (std::uint32_t byte = 0u; byte < 4u; ++byte) {
            Run(*device, Shaders[byte]);
            Check(byte);
        }
        std::puts("sdwa cvt f32 ubyte select tests passed");
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

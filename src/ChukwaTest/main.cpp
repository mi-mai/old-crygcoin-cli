// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#undef NDEBUG

#include <iostream>
#include <chrono>
#include <assert.h>

#include <cxxopts.hpp>
#include <config/CliHeader.h>

#include "CryptoNote.h"
#include "CryptoTypes.h"
#include "Common/StringTools.h"
#include "crypto/crypto.h"

#include <Utilities/ColouredMsg.h>

#define PERFORMANCE_ITERATIONS  1000
#define PERFORMANCE_ITERATIONS_LONG_MULTIPLIER 10

using namespace Crypto;
using namespace CryptoNote;

const std::string INPUT_DATA = "0100fb8e8ac805899323371bb790db19218afd8db8e3755d8b90f39b3d5506a9abce4fa912244500000000ee8146d49fa93ee724deb57d12cbc6c6f3b924d946127c7a97418f9348828f0f02";

const std::string CN_FAST_HASH = "b542df5b6e7f5f05275c98e7345884e2ac726aeeb07e03e44e0389eb86cd05f0";

const std::string CN_SLOW_HASH_V0 = "1b606a3f4a07d6489a1bcd07697bd16696b61c8ae982f61a90160f4e52828a7f";
const std::string CN_SLOW_HASH_V1 = "c9fae8425d8688dc236bcdbc42fdb42d376c6ec190501aa84b04a4b4cf1ee122";
const std::string CN_SLOW_HASH_V2 = "871fcd6823f6a879bb3f33951c8e8e891d4043880b02dfa1bb3be498b50e7578";

const std::string CN_LITE_SLOW_HASH_V0 = "28a22bad3f93d1408fca472eb5ad1cbe75f21d053c8ce5b3af105a57713e21dd";
const std::string CN_LITE_SLOW_HASH_V1 = "87c4e570653eb4c2b42b7a0d546559452dfab573b82ec52f152b7ff98e79446f";
const std::string CN_LITE_SLOW_HASH_V2 = "b7e78fab22eb19cb8c9c3afe034fb53390321511bab6ab4915cd538a630c3c62";

const std::string CN_DARK_SLOW_HASH_V0 = "bea42eadd78614f875e55bb972aa5ec54a5edf2dd7068220fda26bf4b1080fb8";
const std::string CN_DARK_SLOW_HASH_V1 = "d18cb32bd5b465e5a7ba4763d60f88b5792f24e513306f1052954294b737e871";
const std::string CN_DARK_SLOW_HASH_V2 = "a18a14d94efea108757a42633a1b4d4dc11838084c3c4347850d39ab5211a91f";

const std::string CN_DARK_LITE_SLOW_HASH_V0 = "faa7884d9c08126eb164814aeba6547b5d6064277a09fb6b414f5dbc9d01eb2b";
const std::string CN_DARK_LITE_SLOW_HASH_V1 = "c75c010780fffd9d5e99838eb093b37c0dd015101c9d298217866daa2993d277";
const std::string CN_DARK_LITE_SLOW_HASH_V2 = "fdceb794c1055977a955f31c576a8be528a0356ee1b0a1f9b7f09e20185cda28";

const std::string CN_TURTLE_SLOW_HASH_V0 = "546c3f1badd7c1232c7a3b88cdb013f7f611b7bd3d1d2463540fccbd12997982";
const std::string CN_TURTLE_SLOW_HASH_V1 = "29e7831780a0ab930e0fe3b965f30e8a44d9b3f9ad2241d67cfbfea3ed62a64e";
const std::string CN_TURTLE_SLOW_HASH_V2 = "fc67dfccb5fc90d7855ae903361eabd76f1e40a22a72ad3ef2d6ad27b5a60ce5";

const std::string CN_TURTLE_LITE_SLOW_HASH_V0 = "5e1891a15d5d85c09baf4a3bbe33675cfa3f77229c8ad66c01779e590528d6d3";
const std::string CN_TURTLE_LITE_SLOW_HASH_V1 = "ae7f864a7a2f2b07dcef253581e60a014972b9655a152341cb989164761c180a";
const std::string CN_TURTLE_LITE_SLOW_HASH_V2 = "b2172ec9466e1aee70ec8572a14c233ee354582bcb93f869d429744de5726a26";

const std::string CN_CHUKWA = "c8c098976b99f7a22ee904e69bcedd2baad26603af3396263f332fa140840485";

const std::string CN_SOFT_SHELL_V0[] = {
  "5e1891a15d5d85c09baf4a3bbe33675cfa3f77229c8ad66c01779e590528d6d3",
  "e1239347694df77cab780b7ec8920ec6f7e48ecef1d8c368e06708c08e1455f1",
  "118a03801c564d12f7e68972419303fe06f7a54ab8f44a8ce7deafbc6b1b5183",
  "8be48f7955eb3f9ac2275e445fe553f3ef359ea5c065cde98ff83011f407a0ec",
  "d33da3541960046e846530dcc9872b1914a62c09c7d732bff03bec481866ae48",
  "8be48f7955eb3f9ac2275e445fe553f3ef359ea5c065cde98ff83011f407a0ec",
  "118a03801c564d12f7e68972419303fe06f7a54ab8f44a8ce7deafbc6b1b5183",
  "e1239347694df77cab780b7ec8920ec6f7e48ecef1d8c368e06708c08e1455f1",
  "5e1891a15d5d85c09baf4a3bbe33675cfa3f77229c8ad66c01779e590528d6d3",
  "e1239347694df77cab780b7ec8920ec6f7e48ecef1d8c368e06708c08e1455f1",
  "118a03801c564d12f7e68972419303fe06f7a54ab8f44a8ce7deafbc6b1b5183",
  "8be48f7955eb3f9ac2275e445fe553f3ef359ea5c065cde98ff83011f407a0ec",
  "d33da3541960046e846530dcc9872b1914a62c09c7d732bff03bec481866ae48",
  "8be48f7955eb3f9ac2275e445fe553f3ef359ea5c065cde98ff83011f407a0ec",
  "118a03801c564d12f7e68972419303fe06f7a54ab8f44a8ce7deafbc6b1b5183",
  "e1239347694df77cab780b7ec8920ec6f7e48ecef1d8c368e06708c08e1455f1",
  "5e1891a15d5d85c09baf4a3bbe33675cfa3f77229c8ad66c01779e590528d6d3"
};

const std::string CN_SOFT_SHELL_V1[] = {
  "ae7f864a7a2f2b07dcef253581e60a014972b9655a152341cb989164761c180a",
  "ce8687bdd08c49bd1da3a6a74bf28858670232c1a0173ceb2466655250f9c56d",
  "ddb6011d400ac8725995fb800af11646bb2fef0d8b6136b634368ad28272d7f4",
  "02576f9873dc9c8b1b0fc14962982734dfdd41630fc936137a3562b8841237e1",
  "d37e2785ab7b3d0a222940bf675248e7b96054de5c82c5f0b141014e136eadbc",
  "02576f9873dc9c8b1b0fc14962982734dfdd41630fc936137a3562b8841237e1",
  "ddb6011d400ac8725995fb800af11646bb2fef0d8b6136b634368ad28272d7f4",
  "ce8687bdd08c49bd1da3a6a74bf28858670232c1a0173ceb2466655250f9c56d",
  "ae7f864a7a2f2b07dcef253581e60a014972b9655a152341cb989164761c180a",
  "ce8687bdd08c49bd1da3a6a74bf28858670232c1a0173ceb2466655250f9c56d",
  "ddb6011d400ac8725995fb800af11646bb2fef0d8b6136b634368ad28272d7f4",
  "02576f9873dc9c8b1b0fc14962982734dfdd41630fc936137a3562b8841237e1",
  "d37e2785ab7b3d0a222940bf675248e7b96054de5c82c5f0b141014e136eadbc",
  "02576f9873dc9c8b1b0fc14962982734dfdd41630fc936137a3562b8841237e1",
  "ddb6011d400ac8725995fb800af11646bb2fef0d8b6136b634368ad28272d7f4",
  "ce8687bdd08c49bd1da3a6a74bf28858670232c1a0173ceb2466655250f9c56d",
  "ae7f864a7a2f2b07dcef253581e60a014972b9655a152341cb989164761c180a"
};

const std::string CN_SOFT_SHELL_V2[] = {
  "b2172ec9466e1aee70ec8572a14c233ee354582bcb93f869d429744de5726a26",
  "b2623a2b041dc5ae3132b964b75e193558c7095e725d882a3946aae172179cf1",
  "141878a7b58b0f57d00b8fc2183cce3517d9d68becab6fee52abb3c1c7d0805b",
  "4646f9919791c28f0915bc0005ed619bee31d42359f7a8af5de5e1807e875364",
  "3fedc7ab0f8d14122fc26062de1af7a6165755fcecdf0f12fa3ccb3ff63629d0",
  "4646f9919791c28f0915bc0005ed619bee31d42359f7a8af5de5e1807e875364",
  "141878a7b58b0f57d00b8fc2183cce3517d9d68becab6fee52abb3c1c7d0805b",
  "b2623a2b041dc5ae3132b964b75e193558c7095e725d882a3946aae172179cf1",
  "b2172ec9466e1aee70ec8572a14c233ee354582bcb93f869d429744de5726a26",
  "b2623a2b041dc5ae3132b964b75e193558c7095e725d882a3946aae172179cf1",
  "141878a7b58b0f57d00b8fc2183cce3517d9d68becab6fee52abb3c1c7d0805b",
  "4646f9919791c28f0915bc0005ed619bee31d42359f7a8af5de5e1807e875364",
  "3fedc7ab0f8d14122fc26062de1af7a6165755fcecdf0f12fa3ccb3ff63629d0",
  "4646f9919791c28f0915bc0005ed619bee31d42359f7a8af5de5e1807e875364",
  "141878a7b58b0f57d00b8fc2183cce3517d9d68becab6fee52abb3c1c7d0805b",
  "b2623a2b041dc5ae3132b964b75e193558c7095e725d882a3946aae172179cf1",
  "b2172ec9466e1aee70ec8572a14c233ee354582bcb93f869d429744de5726a26"
};

/* Check if we're testing a v1 or v2 hash function */
/* Hacky as fuck lmao */
bool need43BytesOfData(std::string hashFunctionName)
{
    return (hashFunctionName.find("v1") != std::string::npos
        || hashFunctionName.find("v2") != std::string::npos);
}

/* Bit of hackery so we can get the variable name of the passed in function.
   This way we can print the test we are currently performing. */
#define BENCHMARK(hashFunction, iterations) \
   benchmark(hashFunction, #hashFunction, iterations)

template<typename T>
void benchmark(T hashFunction, std::string hashFunctionName, uint64_t iterations)
{
    const BinaryArray& rawData = Common::fromHex(INPUT_DATA);

    if (need43BytesOfData(hashFunctionName) && rawData.size() < 43)
    {
        return;
    }

    Hash hash = Hash();

    auto startTimer = std::chrono::high_resolution_clock::now();

    for (uint64_t i = 0; i < iterations; i++)
    {
        hashFunction(rawData.data(), rawData.size(), hash);
    }

    auto elapsedTime = std::chrono::high_resolution_clock::now() - startTimer;

    std::cout << InformationMsg(hashFunctionName + ": ")
              << SuccessMsg(iterations / std::chrono::duration_cast<std::chrono::seconds>(elapsedTime).count())
              << SuccessMsg(" H/s\n");
}

void benchmark_chukwa(uint64_t iterations, uint64_t argonIterations, uint64_t argonMemoryKB)
{
    const BinaryArray& rawData = Common::fromHex(INPUT_DATA);

    Hash hash = Hash();

    auto startTimer = std::chrono::high_resolution_clock::now();

    for (uint64_t i = 0; i < iterations; i++)
    {
        chukwa_slow_hash_variable(rawData.data(), rawData.size(), hash, argonMemoryKB, argonIterations); 
    }

    auto elapsedTime = std::chrono::high_resolution_clock::now() - startTimer;

    std::cout << std::fixed;

    const double milliseconds = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(elapsedTime).count());

    const uint64_t hashrate = static_cast<uint64_t>((static_cast<double>(iterations) / (milliseconds / (1000 * 1000))));

    std::cout << InformationMsg("Argon2 [" + std::to_string(argonIterations) + " iterations] [" + std::to_string(argonMemoryKB) + " KB memory]: ")
              << SuccessMsg(std::to_string(hashrate) + " H/s\n"); 
}

int main(int argc, char** argv)
{
    const uint64_t o_iterations = PERFORMANCE_ITERATIONS;
    const uint64_t o_iterations_long = o_iterations * PERFORMANCE_ITERATIONS_LONG_MULTIPLIER;

    std::cout << InformationMsg("Welcome! Thanks for helping us out by running this benchmark.\n\n")
              << InformationMsg("To begin with, can you close your miner or any other CPU intensive programs you may be running?\n")
              << "This will help us get an accurate assessment of how fast your computer can perform the new hashing algorithm.\n\n"
              << InformationMsg("Hit Enter when you are ready to begin the performance test: ");

    std::string ignore;
    std::getline(std::cin, ignore);

    std::cout << SuccessMsg("Beginning performance test - This will take a while, please wait.\n\n");

    BENCHMARK(cn_slow_hash_v0, o_iterations);
    BENCHMARK(cn_slow_hash_v1, o_iterations);
    BENCHMARK(cn_slow_hash_v2, o_iterations);

    BENCHMARK(cn_lite_slow_hash_v0, o_iterations);
    BENCHMARK(cn_lite_slow_hash_v1, o_iterations);
    BENCHMARK(cn_lite_slow_hash_v2, o_iterations);

    BENCHMARK(cn_dark_slow_hash_v0, o_iterations);
    BENCHMARK(cn_dark_slow_hash_v1, o_iterations);
    BENCHMARK(cn_dark_slow_hash_v2, o_iterations);

    BENCHMARK(cn_dark_lite_slow_hash_v0, o_iterations);
    BENCHMARK(cn_dark_lite_slow_hash_v1, o_iterations);
    BENCHMARK(cn_dark_lite_slow_hash_v2, o_iterations);

    BENCHMARK(cn_turtle_slow_hash_v0, o_iterations_long);
    BENCHMARK(cn_turtle_slow_hash_v1, o_iterations_long);
    BENCHMARK(cn_turtle_slow_hash_v2, o_iterations_long);

    BENCHMARK(cn_turtle_lite_slow_hash_v0, o_iterations_long);
    BENCHMARK(cn_turtle_lite_slow_hash_v1, o_iterations_long);
    BENCHMARK(cn_turtle_lite_slow_hash_v2, o_iterations_long);

    for (int memoryKB = 8; memoryKB <= 4096; memoryKB *= 2)
    {
        for (int iterations = 1; iterations <= 4; iterations++)
        {
            benchmark_chukwa((o_iterations_long * 500) / (memoryKB * iterations), iterations, memoryKB);
        }
    }

    std::cout << SuccessMsg("\nTest complete. Thanks for waiting.\n")
              << SuccessMsg("Could your post the full output of this test, along with your CPU model, and Operating System?\n\n")
              << InformationMsg("Post the info here: https://github.com/turtlecoin/turtlecoin/issues/715\n\n\n")
              << "Hit Enter to exit: ";

    std::getline(std::cin, ignore);
}

#include <iostream>

#include "wave/file.h"
#include "spleeter/spleeter.h"

void writeStem(const std::vector<float>& data, std::string name)
{
    wave::File file;
    file.Open(name, wave::kOut);
    file.set_sample_rate(44100);
    file.set_channel_number(2);
    file.Write(data);
}

int main(int argc, char* argv[])
{
    if (argc != 4 || std::strcmp(argv[1], "-n"))
    {
        std::cout << "Usage: " << argv[0] << " -n X music.wav" << std::endl;
        std::cout << "-n X specifies how many components to split the input into (valid values: 2, 4, 5)" << std::endl;

        return 1;
    }

    size_t num_stems = 0;
    try
    {
        num_stems = std::stoi(argv[2]);
    }
    catch (const std::exception& e)
    {
        std::cout << "Had exception in parsing argument '" << argv[2] << "', error: " << e.what() << std::endl;

        return 1;
    }

    spleeter::SeparationType stem_count;
    switch (num_stems)
    {
    case 2:
    {
        stem_count = spleeter::TwoStems;
    } break;
    case 4:
    {
        stem_count = spleeter::FourStems;
    } break;
    case 5:
    {
        stem_count = spleeter::FiveStems;
    } break;
    default:
    {
        std::cout << "Invalid number of stems (" << num_stems << ") specified, currently only 2, 4, or 5 are supported." << std::endl;

        return 1;
    }
    }

    // Read wav file
    wave::File input_file;
    auto ret = input_file.Open(argv[3], wave::kIn);
    if (ret != wave::Error::kNoError)
    {
        std::string wave_error{};
        switch (ret)
        {
        case wave::Error::kFailedToOpen:
        {
            wave_error = "failed to open file";
        } break;
        case wave::Error::kNotOpen:
        {
            wave_error = "file not open";
        } break;
        case wave::Error::kInvalidFormat:
        {
            wave_error = "invalid format";
        } break;
        case wave::Error::kWriteError:
        {
            wave_error = "write error";
        } break;
        case wave::Error::kReadError:
        {
            wave_error = "read error";
        } break;
        case wave::Error::kInvalidSeek:
        {
            wave_error = "invalid seek";
        } break;
        default:
        {
            wave_error = "unknown error (raw error value: " + std::to_string(ret) + ")";
        } break;
        }

        std::cout << "Encountered error opening input file (" << argv[3] << "). Error: " << wave_error << std::endl;

        return 1;
    }
    std::vector<float> data{};
    input_file.Read(&data);
    auto source = Eigen::Map<spleeter::Waveform>(data.data(), 2, data.size() / 2);

    std::error_code err{};
    spleeter::Initialize(std::string(SPLEETER_MODELS), {stem_count}, err);
    if (err)
    {
        std::cout << "Encountered error in spleeter initialization: " << err << std::endl;

        return 1;
    }

    // Note: must use () to call the size_t ctor instead of the std::initializer_list ctor
    std::vector<spleeter::Waveform> stems(num_stems);
    switch (num_stems)
    {
    case 2:
    {
        spleeter::Split(source, &stems[0], &stems[1], err);
    } break;
    case 4:
    {
        spleeter::Split(source, &stems[0], &stems[1], &stems[2], &stems[3], err);
    } break;
    case 5:
    {
        spleeter::Split(source, &stems[0], &stems[1], &stems[2], &stems[3], &stems[4], err);
    } break;
    default:
    {
        // Shouldn't be possible, should have been caught above
        return 1;
    }
    }

    if (err)
    {
        std::cout << "Encountered error while spleeting input: " << err << std::endl;

        return 1;
    }

    std::vector<std::string> component_names{"vocals", "drums", "bass", "piano"};
    for (size_t i = 0; i < stems.size(); i++)
    {
        std::string component_name{"other"};
        // Special case: the last stem is always "other", the rest are consistent
        if (i != stems.size() - 1)
        {
            component_name = component_names[i];
        }

        writeStem({stems[i].data(), stems[i].data() + stems[i].size()}, std::string{"./" + component_name + ".wav"});
    }

    return 0;
}

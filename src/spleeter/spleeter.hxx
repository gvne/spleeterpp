namespace spleeter {

namespace internal {

void Initialize(const std::string &path_to_model, SeparationType type);

template <SeparationType... types> struct Initializer {};

template <SeparationType first, SeparationType... other>
struct Initializer<first, other...> {
  static void Run(const std::string &path_to_models) {
    Initializer<first>::Run(path_to_models);
    Initializer<other...>::Run(path_to_models);
  }
};

template <> struct Initializer<TwoStems> {
  static void Run(const std::string &path_to_models) {
    Initialize(path_to_models + "/2stems", TwoStems);
  }
};

template <> struct Initializer<FourStems> {
  static void Run(const std::string &path_to_models) {
    Initialize(path_to_models + "/4stems", FourStems);
  }
};

template <> struct Initializer<FiveStems> {
  static void Run(const std::string &path_to_models) {
    Initialize(path_to_models + "/5stems", FiveStems);
  }
};

template <SeparationType type> struct Splitter {};

template <> struct Splitter<TwoStems> {
  static void Run(const Waveform &input, Waveform *vocals,
                  Waveform *accompaniment) {
    Split(input, vocals, accompaniment);
  }
};

template <> struct Splitter<FourStems> {
  void Run(const Waveform &input, Waveform *vocals, Waveform *drums,
           Waveform *bass, Waveform *other) {
    Split(input, vocals, drums, bass, other);
  }
};

template <>
struct Splitter<FiveStems> {
  static void Run(const Waveform &input, Waveform *vocals, Waveform *drums,
                  Waveform *bass, Waveform *piano, Waveform *other) {
    Split(input, vocals, drums, bass, piano, other);
  }
};

} // namespace internal

template <SeparationType... types>
void Initialize(const std::string &path_to_models) {
  internal::Initializer<types...>::Run(path_to_models);
}

template <SeparationType type, typename... T>
void Split(const Waveform &input, T *... outputs) {
  internal::Splitter<type>::Run(input, outputs...);
}

} // namespace spleeter

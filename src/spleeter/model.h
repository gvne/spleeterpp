namespace spleeter {
  
  void RunModel(const Waveform &input, SeparationType separation_type,
                const std::vector<std::string>& output_names,
                std::vector<TFHandlePtr<TF_Tensor>> *result,
                std::error_code &err);
  void SetOutput(const std::vector<TFHandlePtr<TF_Tensor>> &tf_output,
                 uint64_t frame_count, std::vector<Waveform *> output);

}  // namespace spleeter

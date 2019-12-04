#include <iostream>
#include "spleeter++/spleeter++.h"

int main(int argc, char* argv[]) {
  // Find the Resource folder
  // see https://stackoverflow.com/questions/10364877/c-how-to-remove-filename-from-path-string/10364907
  std::string bin_path(argv[0]);
  auto bin_folder = bin_path.substr(0, bin_path.find_last_of("\\/"));
  auto environment_path = bin_folder + "/../Resources";
  std::cout << environment_path << std::endl;
  
  // Initialize spleeter
  spleeter::Initialize(environment_path);
  // Extract
  spleeter::Separate(std::string(TEST_FILE_PATH), "vocals.wav", "other.wav");
  
  return 0;
}

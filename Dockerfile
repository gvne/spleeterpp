FROM continuumio/anaconda3

MAINTAINER Loreto Parisi loretoparisi@gmail.com

RUN apt-get update && apt-get install -y \
    wget \
    unzip \
    rsync \
    gcc \
    build-essential \
    software-properties-common \
    cmake

# spleeterpp source
WORKDIR spleeterpp
COPY . .

# spleeterpp build
RUN mkdir build && cd build && \
    cmake -Dspleeter_regenerate_models=ON .. && \
    cmake --build .

# defaults command
CMD ["bash"]

FROM continuumio/anaconda3

MAINTAINER Loreto Parisi loretoparisi@gmail.com

WORKDIR spleeter

RUN apt-get update && apt-get install -y \
    wget \
    unzip \
    rsync \
    gcc \
    build-essential \
    software-properties-common

# spleeterpp source
COPY . spleeter/

# bazel install
RUN bash spleeter/bazel-0.25.2-installer-linux-x86_64.sh
# RUN wget https://github.com/bazelbuild/bazel/releases/download/0.25.2/bazel-0.25.2-installer-linux-x86_64.sh
# RUN bash bazel-0.25.2-installer-linux-x86_64.sh --user

# tensorflow bazel build
RUN git clone https://github.com/tensorflow/tensorflow.git && \
    cd tensorflow && \
    git checkout v1.14.0 && \
    rm BUILD
RUN cd tensorflow && \
    python tensorflow/tools/pip_package/setup.py install && \
    mv build build-bu && \
    git checkout BUILD && \
    ./configure && \
    bazel build --config=monolithic --jobs=6 --verbose_failures //tensorflow:libtensorflow_cc.so

# tensorflow install
ENV INSTALL_DIR=install
ENV INCLUDE_DIR=$INSTALL_DIR/include
RUN cd tensorflow && \
    mkdir -p $INSTALL_DIR/bin && \
    cp bazel-bin/tensorflow/libtensorflow_cc.so* $INSTALL_DIR/bin/ && \
    mkdir -p $INSTALL_DIR/include && \
    rsync -a --prune-empty-dirs --include '*/' --include '*.h' --exclude '*' tensorflow/ $INCLUDE_DIR/tensorflow && \
    mkdir -p $INSTALL_DIR/include/third_party/eigen3/unsupported/ && \
    cp -r ./bazel-tensorflow/external/eigen_archive/unsupported/Eigen $INSTALL_DIR/include/third_party/eigen3/unsupported/Eigen && \
    cp -r ./bazel-tensorflow/external/eigen_archive/Eigen $INSTALL_DIR/include/third_party/eigen3/Eigen

# spleeterpp build
RUN cd spleeter && \
    mkdir build && cd build && \
    cmake -DTENSORFLOW_CC_INSTALL_DIR=$INSTALL_DIR .. && \
    cmake --build .

# defaults command
CMD ["bash"]
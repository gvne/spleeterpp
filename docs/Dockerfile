FROM ubuntu:18.04
RUN apt-get update
RUN apt-get install -y python-dev python-pip doxygen
COPY docs/requirements.txt .
RUN pip install -r requirements.txt
RUN pip install sphinx_rtd_theme
WORKDIR /code/docs

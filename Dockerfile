FROM ubuntu:bionic

RUN  apt-get update \
  && apt-get -yq install \
      cmake git libomp-dev g++ imagemagick
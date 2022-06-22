FROM ubuntu:18.04

MAINTAINER Petr Velan <petr.velan@cesnet.cz>

# IPFIXcol dependencies
RUN apt-get update && apt-get install -y \
  autoconf \
  bison \
  build-essential \
  docbook-xsl \
  doxygen \
  flex \
  git \
  liblzo2-dev \
  libtool \
  libsctp-dev \
  libssl1.0-dev \
  libxml2 \
  libxml2-dev \
  pkg-config \
  xsltproc

RUN apt-get install -y gcc g++ cmake make python3-docutils zlib1g-dev librdkafka-dev liblz4-dev libzstd-dev

WORKDIR /root
RUN git clone https://github.com/CESNET/libfds.git
WORKDIR /root/libfds
RUN mkdir build
WORKDIR /root/libfds/build
RUN cmake .. -DCMAKE_INSTALL_PREFIX=/usr
RUN make
RUN make install

# checkout IPFIXcol
WORKDIR /root/
RUN git clone https://github.com/CESNET/ipfixcol2.git
WORKDIR /root/ipfixcol2/
RUN mkdir build
WORKDIR /root/ipfixcol2/build
RUN cmake ..
RUN make
RUN make install

EXPOSE 4739 4739/udp
COPY config/ipfixcol2.xml /etc/ipfixcol2.xml
CMD bash


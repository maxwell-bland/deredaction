FROM ubuntu:20.04

ENV TZ=US/Chicago
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

WORKDIR /app/
RUN apt-get update;
RUN apt-get install -y tzdata
RUN apt-get -y install openjdk-16-jdk 
RUN apt-get -y install imagemagick 
RUN apt-get -y install ruby-full 
RUN apt-get -y install python3 
RUN apt-get -y install awscli 
RUN apt-get -y install pdftk 
RUN apt-get -y install ghostscript 
RUN apt-get -y install libpoppler-dev
RUN apt-get -y install python3-pip
RUN apt-get -y install build-essential
RUN apt-get -y install cmake
RUN apt-get -y install debhelper
RUN apt-get -y install dpkg
RUN apt-get -y install gobject-introspection
RUN apt-get -y install gtk-doc-tools
RUN apt-get -y install libboost-dev
RUN apt-get -y install libcairo2-dev
RUN apt-get -y install libfontconfig1-dev
RUN apt-get -y install libfreetype6-dev
RUN apt-get -y install libgirepository1.0-dev
RUN apt-get -y install libglib2.0-dev
RUN apt-get -y install libgtk-3-dev
RUN apt-get -y install libjpeg-dev
RUN apt-get -y install liblcms2-dev
RUN apt-get -y install libnss3-dev
RUN apt-get -y install libopenjp2-7-dev
RUN apt-get -y install libpng-dev
RUN apt-get -y install libtiff-dev
RUN apt-get -y install pkg-config
RUN apt-get -y install python3
RUN apt-get -y install qtbase5-dev
RUN apt-get -y install zlib1g-dev
RUN apt-get -y install libcairo2-doc
RUN apt-get -y install libglib2.0-doc
RUN apt-get -y install python3-pip
RUN apt-get -y install python3-numpy

COPY . .
WORKDIR /app/PDF-Text-Analysis/lib/poppler
RUN mkdir build
WORKDIR /app/PDF-Text-Analysis/lib/poppler/build
RUN cmake ..
RUN make
RUN make install
WORKDIR /app/PDF-Text-Analysis/adjustment
RUN make VectorString.o
WORKDIR /app/PDF-Text-Analysis/
RUN make pts

RUN pip3 install PyMuPDF
WORKDIR /app/redaction-defenses
RUN cp -r ../PDF-Text-Analysis .

RUN apt-get -y install openjdk-17-*

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get -y install locales
RUN echo "en_US.UTF-8 UTF-8" > /etc/locale.gen
RUN locale-gen

RUN sed -i '/disable ghostscript format types/,+6d' /etc/ImageMagick-6/policy.xml

CMD [ "bash" ]

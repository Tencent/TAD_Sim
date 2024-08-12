############################################################
# Dockerfile 文件
############################################################
FROM ubuntu:18.04

RUN apt-get update && apt-get \
    install -y build-essential cmake git libssl-dev wget\
    libproj-dev libsqlite3-dev libspatialite-dev libcurl4-openssl-dev

RUN wget https://www.python.org/ftp/python/3.6.12/Python-3.6.12.tgz \
    && tar xf Python-3.6.12.tgz \
    && cd Python-3.6.12 \
    && ./configure && make -j8 && make install \
    && cd .. \
    && rm -fr Python-3.6.*

# config pip
RUN echo "[global]\nindex-url = https://mirrors.tencent.com/tencent_pypi/simple" > /etc/pip.conf

# upgrade pip
RUN pip3 install -U pip && pip3 install wheel

RUN pip3 install -U qci_worker -i https://mirrors.tencent.com/tencent_pypi/simple

WORKDIR /workdir/

CMD ["bash"]

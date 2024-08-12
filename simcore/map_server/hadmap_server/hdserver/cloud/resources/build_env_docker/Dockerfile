FROM csighub.tencentyun.com/tad-autocloud/hadmap_server_cloud_build:v0.2
ENV http_proxy=
ENV https_proxy=
ENV HTTPS_PROXY=
ENV HTTP_PROXY=
#RUN pip3 install -U qci_worker -i https://mirrors.tencent.com/tencent_pypi/simple
RUN echo '\n#ppa for python3\n' >> /etc/apt/sources.list
RUN echo 'deb http://ppa.launchpad.net/deadsnakes/ppa/ubuntu xenial main' >> /etc/apt/sources.list
RUN echo 'deb-src http://ppa.launchpad.net/deadsnakes/ppa/ubuntu xenial main' >> /etc/apt/sources.list
RUN apt-key adv --keyserver keyserver.ubuntu.com --recv-keys F23C5A6CF475977595C89F51BA6932366A755776
#
RUN apt-get update && apt-get upgrade -y && apt-get install -y --no-install-recommends \
          curl \
          net-tools \
          vim-tiny \
          python3.8 \
          python3.8-distutils \
          python3.8-venv \
          && update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.8 1 \
          && apt-get clean \
          && rm -fr /var/lib/apt/lists/*
#RUN apt-get update && apt-get install -y libxerces-c-dev
RUN curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py  && /usr/bin/python3 get-pip.py
RUN pip3 install -U qci_worker -i https://mirrors.tencent.com/tencent_pypi/simple

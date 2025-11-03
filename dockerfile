FROM ubuntu:24.04

RUN mkdir /hypatia

COPY . /hypatia

WORKDIR /hypatia

RUN apt-get update
RUN apt-get install -y sudo git

RUN bash hypatia_install_dependencies_docker.sh
RUN bash hypatia_build.sh
# RUN bash hypatia_run_tests.sh

CMD /bin/bash
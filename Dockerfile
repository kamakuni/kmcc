FROM ubuntu
RUN mkdir /src
ADD ./src /src
WORKDIR /src
RUN apt-get update && apt-get -y install build-essential && apt-get install -y vim && apt-get install -y less
CMD ["/bin/bash"]

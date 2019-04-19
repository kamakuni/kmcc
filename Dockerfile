FROM ubuntu
RUN mkdir /src
ADD ./src /src
WORKDIR /src
RUN apt-get update && apt-get -y install build-essential
CMD ["/bin/bash"]

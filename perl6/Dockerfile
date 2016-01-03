FROM ubuntu:vivid
MAINTAINER Joel Martin <github@martintribe.org>

ENV PATH="~/.rakudobrew/bin:$PATH"

RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y \
        gcc \
        git \
        libreadline-dev \
        make \
 && git clone --depth 1 https://github.com/tadzik/rakudobrew ~/.rakudobrew \
 && PATH=~/.rakudobrew/bin:$PATH rakudobrew build moar \
 && true

#    rakudobrew build-panda && \

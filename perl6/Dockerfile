FROM rakudo-star

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
        libreadline-dev \
    && echo ok.

WORKDIR /run
CMD /bin/bash

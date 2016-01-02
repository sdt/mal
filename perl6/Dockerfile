FROM rakudo-star

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
        libreadline-dev \
    && echo ok.

WORKDIR /run

ENV INPUTRC=$PWD/.inputrc \
    MAL_HISTORY=$PWD/.mal_history \
    PERL6LIB=$PWD

CMD /bin/bash

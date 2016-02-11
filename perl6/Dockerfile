FROM rakudo-star
MAINTAINER Joel Martin <github@martintribe.org>

ARG APT_MIRROR=http://httpredir.debian.org

RUN sed -i -e "s!http://httpredir\.debian\.org!$APT_MIRROR!" \
        /etc/apt/sources.list \
 && apt-get update \
 && buildDeps='gcc make' \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y $buildDeps \
 && /usr/share/perl6/site/bin/panda install Linenoise \
 && apt-get purge -y --auto-remove $buildDeps \
 && rm -rf /var/lib/apt/lists/* \
 && true

CMD ["perl6"]

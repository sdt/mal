#!/bin/bash

IMAGE_NAME=mal-perl6
CONTAINER_NAME=mal-perl6-running

WORKDIR=/run

run() {
    docker rm -f $CONTAINER_NAME > /dev/null 2>/dev/null
    docker run \
      -e PERL6LIB=$WORKDIR \
      -v $PWD:$WORKDIR \
      -w $WORKDIR \
      -ti --rm --name $CONTAINER_NAME $IMAGE_NAME "$@"
}

case $1 in

    build)
        if [[ -n $APT_MIRROR ]]; then
            ARGS="--build-arg APT_MIRROR=$APT_MIRROR"
        else
            echo set APT_MIRROR to use a local debian repo.
        fi
        docker build $ARGS -t $IMAGE_NAME .
        ;;

    run)
        shift
        run "$@"
        ;;

    *)
        echo "usage: $0 [build|run]"
        exit 1

        ;;

esac


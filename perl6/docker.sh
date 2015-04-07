#!/bin/bash

IMAGE_NAME=mal-perl6
CONTAINER_NAME=mal-perl6-running

WORKDIR=/run

run() {
    docker rm -f $CONTAINER_NAME > /dev/null 2>/dev/null
    docker run \
      -v $PWD:$WORKDIR \
      -e INPUTRC=$WORKDIR/.inputrc \
      -e MAL_HISTORY=$WORKDIR/.mal_history \
      -ti --name $CONTAINER_NAME $IMAGE_NAME "$@"
}

case $1 in

    build)
        docker build -t $IMAGE_NAME .
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


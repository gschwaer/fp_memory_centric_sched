#!/bin/bash
project_name="dynamic_prem"
build_name="rt_druid_gh65"

parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
mkdir -p $parent_path/.eclipse
docker run \
       --rm \
       -v /tmp/.X11-unix:/tmp/.X11-unix \
       -v /tmp/.docker.xauth:/tmp/.docker.xauth \
       -v $parent_path/../code:/home/$USER/code \
       -v $parent_path/.eclipse:/home/$USER/.eclipse \
       -e DISPLAY=$DISPLAY \
       $project_name/$build_name:latest

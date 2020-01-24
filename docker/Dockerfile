# Use the build.sh script to build this dockerfile.
FROM ubuntu:18.04
ARG USERNAME
ARG PASSWORD
ARG ECLIPSE_TOOLCHAIN_ARCHIVE
# update package cache
RUN apt-get update
RUN apt-get upgrade --yes
RUN apt-get update
# setup user
RUN apt-get install --yes --no-install-recommends --no-install-suggests sudo
RUN useradd ${USERNAME} -s /bin/bash -m -G sudo && \
	echo "${USERNAME}:${PASSWORD}" | chpasswd
# install packages
## network related
RUN apt-get install --yes --no-install-recommends --no-install-suggests wget git ca-certificates
## eclipse RT-Druid related
RUN apt-get install --yes --no-install-recommends --no-install-suggests openjdk-8-jre
## copy+extract & chown arm toolchain & setup bashrc
ADD ./${ECLIPSE_TOOLCHAIN_ARCHIVE} /home/${USERNAME}/
ADD ./org.eclipse.ui.ide.prefs /home/${USERNAME}/eclipse/configuration/.settings/org.eclipse.ui.ide.prefs
RUN sed -i "s/_USERNAME_/${USERNAME}/g" /home/${USERNAME}/eclipse/configuration/.settings/org.eclipse.ui.ide.prefs
RUN chown ${USERNAME}:${USERNAME} -R /home/${USERNAME}/*
# install tools
RUN apt-get install --yes --no-install-recommends --no-install-suggests make doxygen gcc-aarch64-linux-gnu
# setup something as unprevileged user (e.g. using make)
#USER ${USERNAME}
#WORKDIR /home/${USERNAME}
#RUN git clone https://github.com/texane/stlink.git && cd stlink && make release
#USER root
# clean package cache
RUN apt-get clean
# setup entry point
USER ${USERNAME}
WORKDIR /home/${USERNAME}
#CMD bash
CMD eclipse/eclipse
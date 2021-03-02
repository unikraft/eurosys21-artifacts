FROM debian_env as opensmtpd

# INSTALL opensmtpd
ENV APP_NAME opensmtpd
ENV APP_VERSION 6.8.0p2
RUN apt-get update && apt-get install -y libevent-dev libssl-dev zlib1g-dev bison

RUN wget https://www.opensmtpd.org/archives/opensmtpd-${APP_VERSION}.tar.gz
RUN tar -xf opensmtpd-${APP_VERSION}.tar.gz

WORKDIR /home/opensmtpd-${APP_VERSION}/
RUN ./configure
RUN make
WORKDIR /home/

RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p /home/opensmtpd-${APP_VERSION}/contrib/libexec/mail.local/mail.local  -o \" -L -f from sample@hotmail.com\" -w 10 &> out.txt" >> run.sh
RUN echo "cat /root/mail.local_output/mail.local.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]
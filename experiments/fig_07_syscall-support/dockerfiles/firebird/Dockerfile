FROM debian_env as firebird

# INSTALL FIREBIRD
ENV APP_NAME firebird
RUN apt-get update && \
    apt-get install --no-install-recommends -y ${APP_NAME}3.0-server

WORKDIR /home/
RUN echo "#!/bin/bash" >> run.sh

RUN echo "./tools --dep -p /usr/sbin/fbguard -o \" -pidfile /run/firebird3.0/default.pid\" -t /home/testfiles/test_firebird.json &> out.txt" >> run.sh
RUN echo "cat /root/fbguard_output/fbguard.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]
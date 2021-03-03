FROM debian_env as avahi

# INSTALL AVAHI
ENV APP_NAME avahi
RUN apt-get update && apt-get install -y avahi-daemon avahi-utils dnsutils bind9utils

WORKDIR /home/
RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p /usr/sbin/avahi-daemon -t /home/testfiles/test_avahi.json &> out.txt " >> run.sh
RUN echo "cat /root/avahi-daemon_output/avahi-daemon.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]
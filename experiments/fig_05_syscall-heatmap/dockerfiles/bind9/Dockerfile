FROM debian_env as bind9

# INSTALL BIND9
ENV APP_NAME bind9
RUN apt-get update && apt-get install -y bind9 dnsutils bind9utils

WORKDIR /home/
RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p named -o \" -g\" -t /home/testfiles/test_bind9.json &> out.txt " >> run.sh
RUN echo "cat /root/named_output/named.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]
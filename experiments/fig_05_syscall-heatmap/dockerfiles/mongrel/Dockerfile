FROM debian_env as mongrel

# INSTALL MONGREL
ENV APP_NAME mongrel
ENV APP_VERSION 1.9.0
RUN apt-get update && apt-get install -y make gcc libsqlite3-dev ca-certificates sqlite3 libtool libzmq3-dev

RUN wget https://github.com/mongrel2/mongrel2/releases/download/v1.9.0/mongrel2-v${APP_VERSION}.tar.gz
RUN tar -xzvf mongrel2-v${APP_VERSION}.tar.gz

WORKDIR /home/mongrel2-v${APP_VERSION}
RUN sed -i 's/typedef/\/\//' /home/mongrel2-v${APP_VERSION}/src/mem/align.h
RUN make clean bin/mongrel2 m2sh procer

WORKDIR /home/
RUN cp /home/mongrel2-v${APP_VERSION}/bin/mongrel2 /usr/bin/mongrel2
RUN cp /home/mongrel2-v${APP_VERSION}/tools/m2sh/build/m2sh /usr/bin/m2sh
RUN cp /home/mongrel2-v${APP_VERSION}/examples/configs/sample.conf mysite.conf
RUN m2sh load -config mysite.conf
RUN mkdir -p ../run/ .//logs/ .//tmp/

RUN sed -i 's/80/6767/' testfiles/test_http.json
RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p m2sh -o \" start -host localhost\" -t /home/testfiles/test_http.json &> out.txt" >> run.sh
RUN echo "cat /root/m2sh_output/m2sh.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]
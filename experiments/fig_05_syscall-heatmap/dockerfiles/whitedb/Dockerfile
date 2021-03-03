FROM debian_env as whitedb

# INSTALL WHITEDB
ENV APP_NAME whitedb
ENV VERSION 0.7.3
RUN apt-get update && \
    apt-get install --no-install-recommends -y ${APP_NAME} gcc libwgdb-dev

RUN curl -OL http://whitedb.org/whitedb-${VERSION}.tar.gz
RUN tar -zxvf whitedb-${VERSION}.tar.gz
WORKDIR /home/whitedb-${VERSION}/
RUN ./configure && make && make install
RUN gcc -g -o Examples/mydemo Examples/demo.c -lwgdb
RUN cp Examples/mydemo /home/${APP_NAME} 

WORKDIR /home
RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p ${APP_NAME} -w 10  &> out.txt" >> run.sh
RUN echo "cat /root/${APP_NAME}_output/${APP_NAME}.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]
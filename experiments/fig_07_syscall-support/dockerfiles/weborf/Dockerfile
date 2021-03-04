FROM debian_env as weborf

# INSTALL WEBORF
ENV APP_NAME weborf
ENV VERSION 0.17
RUN apt-get update && \
    apt-get install --no-install-recommends -y gcc make

RUN curl -OL https://github.com/ltworf/weborf/releases/download/${VERSION}/weborf_${VERSION}.orig.tar.gz
RUN tar -zxvf weborf_${VERSION}.orig.tar.gz
WORKDIR /home/weborf-${VERSION}
RUN ./configure && make && make install 

WORKDIR /home/
RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p weborf -o \" -p 80\" -t /home/testfiles/test_http.json &> out.txt" >> run.sh
RUN echo "cat /root/${APP_NAME}_output/${APP_NAME}.json" >> run.sh
RUN chmod +x run.sh

COPY ./mime.types /etc/mime.types

CMD ["./run.sh"]